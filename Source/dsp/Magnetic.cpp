#include "Magnetic.h"
#include "../core/Params.h"
#include "../core/MacroController.h"

namespace ReallyCheap
{

Magnetic::Magnetic()
{
    random.setSeedRandomly();
}

void Magnetic::prepare(double sampleRate_, int blockSize, int numChannels_)
{
    juce::ignoreUnused(blockSize);
    sampleRate = sampleRate_;
    numChannels = numChannels_;
    
    channels.clear();
    channels.resize(numChannels);
    
    // Setup parameter smoothing (30ms)
    const double smoothingTime = 0.03;
    smoothedCompAmount.reset(sampleRate, smoothingTime);
    smoothedSatAmount.reset(sampleRate, smoothingTime);
    smoothedCrosstalk.reset(sampleRate, smoothingTime);
    smoothedHeadBump.reset(sampleRate, smoothingTime);
    smoothedWear.reset(sampleRate, smoothingTime);
    
    // Initialize per-channel filters
    for (auto& channel : channels)
    {
        // Pre-emphasis: +6dB/oct above 2kHz for saturation clarity
        auto preEmphCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 2000.0f, 0.707f, juce::Decibels::decibelsToGain(6.0f));
        channel.preEmphasisFilter.coefficients = preEmphCoeffs;
        
        // De-emphasis: -6dB/oct above 2kHz to restore balance
        auto deEmphCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 2000.0f, 0.707f, juce::Decibels::decibelsToGain(-6.0f));
        channel.deEmphasisFilter.coefficients = deEmphCoeffs;
        
        // Head bump: Low-shelf at 80Hz, Q=0.7
        auto headBumpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate, 80.0f, 0.7f, 1.0f); // Gain will be updated per-sample
        channel.headBumpFilter.coefficients = headBumpCoeffs;
        
        // Wear: Low-pass starting at 20kHz (will be updated)
        auto wearCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, 20000.0f);
        channel.wearFilter.coefficients = wearCoeffs;
    }
    
    reset();
}

void Magnetic::reset()
{
    for (auto& channel : channels)
    {
        // Reset compression envelope
        channel.compEnvState1 = 0.0f;
        channel.compEnvState2 = 0.0f;
        channel.lastGainReduction = 0.0f;
        
        // Reset all filters
        channel.preEmphasisFilter.reset();
        channel.deEmphasisFilter.reset();
        channel.headBumpFilter.reset();
        channel.wearFilter.reset();
        
        // Clear crosstalk delay
        std::fill(channel.crosstalkDelay.begin(), channel.crosstalkDelay.end(), 0.0f);
        channel.crosstalkWritePos = 0;
    }
    
    // Reset parameter smoothing
    smoothedCompAmount.setCurrentAndTargetValue(ParameterDefaults::magComp);
    smoothedSatAmount.setCurrentAndTargetValue(ParameterDefaults::magSat);
    smoothedCrosstalk.setCurrentAndTargetValue(ParameterDefaults::magCrosstalk);
    smoothedHeadBump.setCurrentAndTargetValue(ParameterDefaults::magHeadBumpHz);
    smoothedWear.setCurrentAndTargetValue(ParameterDefaults::magWear);
}

void Magnetic::process(juce::AudioBuffer<float>& buffer, 
                      juce::AudioPlayHead* playHead, 
                      juce::AudioProcessorValueTreeState& apvts,
                      const MacroController& macro) noexcept
{
    juce::ignoreUnused(playHead);
    
    const int numSamples = buffer.getNumSamples();
    const int bufferChannels = buffer.getNumChannels();
    
    // Get parameters
    const bool magOn = *apvts.getRawParameterValue(ParameterIDs::magOn) > 0.5f;
    
    if (!magOn)
        return;
    
    const float baseCompAmount = *apvts.getRawParameterValue(ParameterIDs::magComp);
    const float baseSatAmount = *apvts.getRawParameterValue(ParameterIDs::magSat);
    const float crosstalk = *apvts.getRawParameterValue(ParameterIDs::magCrosstalk);
    const float headBump = *apvts.getRawParameterValue(ParameterIDs::magHeadBumpHz);
    const float wear = *apvts.getRawParameterValue(ParameterIDs::magWear);
    
    // Generate hiss level based on wear amount (comprehensive aging control)
    const float hissLevel = wear * wear * 0.15f; // Quadratic scaling for more realistic aging
    
    // Apply macro modulation with guardrails
    const float compAmount = baseCompAmount * macro.magneticCompGain();
    const float satAmount = baseSatAmount * macro.magneticSatGain();
    
    // Update smoothed parameters
    smoothedCompAmount.setTargetValue(compAmount);
    smoothedSatAmount.setTargetValue(satAmount);
    smoothedCrosstalk.setTargetValue(crosstalk);
    smoothedHeadBump.setTargetValue(headBump);
    smoothedWear.setTargetValue(wear);
    
    // Process each channel
    for (int ch = 0; ch < std::min(bufferChannels, static_cast<int>(channels.size())); ++ch)
    {
        auto& channel = channels[ch];
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            float output = input;
            
            // Get smoothed parameter values
            const float currentCompAmount = smoothedCompAmount.getNextValue();
            const float currentSatAmount = smoothedSatAmount.getNextValue();
            const float currentHeadBump = smoothedHeadBump.getNextValue();
            const float currentWear = smoothedWear.getNextValue();
            
            // 1. COMPRESSION - Level-dependent gain reduction
            output = processCompression(channel, output, currentCompAmount);
            
            // 2. SATURATION - Tape-like soft clipping with pre/de-emphasis
            output = processSaturation(channel, output, currentSatAmount);
            
            // 3. HEAD BUMP - Low-shelf boost around 80Hz
            if (sample == 0) // Only update filter coefficients once per block
                updateHeadBumpFilter(channel, currentHeadBump);
            output = channel.headBumpFilter.processSample(output);
            
            // 4. WEAR - Gentle HF rolloff  
            if (sample == 0) // Only update filter coefficients once per block
                updateWearFilter(channel, currentWear);
            output = channel.wearFilter.processSample(output);
            
            // 5. HISS - Tape aging noise (integrated with wear control)
            if (hissLevel > 0.0f)
            {
                // Generate pink-ish noise for realistic tape hiss
                float whiteNoise = (random.nextFloat() - 0.5f) * 2.0f;
                float hissNoise = whiteNoise * hissLevel * 0.02f; // Scale to reasonable level
                
                // High-pass filter the hiss to simulate tape characteristics
                if (ch == 0) // Only filter once per sample 
                {
                    // Simple high-pass to emphasize high frequencies like real tape hiss
                    hissNoise *= (1.0f + currentWear * 0.5f); // More emphasis with more wear
                }
                
                output += hissNoise;
            }
            
            // Safety check for NaN/Inf
            if (!std::isfinite(output))
                output = 0.0f;
                
            channelData[sample] = output;
        }
    }
    
    // 6. CROSSTALK - Apply stereo bleed between channels (post-processing)
    // Note: Using the final smoothed value for the entire buffer for crosstalk
    const float finalCrosstalk = smoothedCrosstalk.getCurrentValue();
    applyCrosstalk(buffer, finalCrosstalk);
}

float Magnetic::processCompression(ChannelState& channel, float input, float compAmount) noexcept
{
    if (compAmount <= 0.0f)
        return input;
    
    // RMS-style envelope following with 2-pole smoothing
    float inputLevel = std::abs(input);
    
    // Faster attack, slower release for more pumping character
    float alpha1 = inputLevel > channel.compEnvState1 ? 0.9f : 0.9995f; // Faster attack, slower release
    channel.compEnvState1 = alpha1 * channel.compEnvState1 + (1.0f - alpha1) * inputLevel;
    
    // Less smoothing for more obvious compression artifacts
    float alpha2 = 0.99f; // Less smoothing for more character
    channel.compEnvState2 = alpha2 * channel.compEnvState2 + (1.0f - alpha2) * channel.compEnvState1;
    
    // EXTREMELY aggressive compression curve for maximum in-your-face effect
    float level = channel.compEnvState2;
    float threshold = 0.05f - compAmount * 0.048f; // Even lower threshold (0.05 to 0.002)
    float ratio = 6.0f + compAmount * 24.0f; // 6:1 to 30:1 ratio - brutally extreme compression
    
    float gainReduction = 0.0f;
    if (level > threshold)
    {
        float overThreshold = level - threshold;
        float compressedOver = overThreshold / ratio;
        gainReduction = overThreshold - compressedOver;
        
        // Allow extreme gain reduction for maximum pumping effect
        gainReduction = juce::jmin(gainReduction, 0.95f); // ~26dB max reduction
    }
    
    // Scale the effect based on compAmount for more control
    gainReduction *= compAmount;
    
    // Less smoothing for more obvious compression pumping
    float smoothingCoeff = 0.999f; // Less smoothing for more aggressive character
    channel.lastGainReduction = smoothingCoeff * channel.lastGainReduction + 
                               (1.0f - smoothingCoeff) * gainReduction;
    
    float compressionGain = 1.0f - channel.lastGainReduction;
    // Add aggressive makeup gain for more presence
    float makeupGain = 1.0f + (channel.lastGainReduction * 1.2f); // Restore 120% of reduced gain for more punch
    float result = input * compressionGain * makeupGain;
    
    // Safety check
    if (!std::isfinite(result))
        return input;
        
    return result;
}

float Magnetic::processSaturation(ChannelState& channel, float input, float satAmount) noexcept
{
    if (satAmount <= 0.0f)
        return input;
    
    // Pre-emphasis for clarity during saturation
    float preEmphasized = channel.preEmphasisFilter.processSample(input);
    
    // Much more drive for obvious saturation
    float drive = 1.0f + satAmount * 9.0f; // Up to 10x drive
    float driven = preEmphasized * drive;
    
    // Soft saturation using tanh
    float saturated = std::tanh(driven * 0.7f) / 0.7f; // Normalized tanh
    
    // Mix with clean signal for subtle effect
    float mixed = input + satAmount * (saturated - input);
    
    // De-emphasis to restore frequency balance
    float result = channel.deEmphasisFilter.processSample(mixed);
    
    // Safety check
    if (!std::isfinite(result))
        return input;
        
    return result;
}

void Magnetic::applyCrosstalk(juce::AudioBuffer<float>& buffer, float crosstalkAmount) noexcept
{
    if (crosstalkAmount <= 0.0f || buffer.getNumChannels() < 2)
        return;
    
    const int numSamples = buffer.getNumSamples();
    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = buffer.getWritePointer(1);
    
    // Scale crosstalk amount (0-40% max) - very obvious effect
    float bleedAmount = crosstalkAmount * 0.4f;
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float leftSample = leftData[sample];
        float rightSample = rightData[sample];
        
        // Add small delay for more realistic tape crosstalk
        auto& leftChannel = channels[0];
        auto& rightChannel = channels[1];
        
        // Store current samples in delay buffers
        leftChannel.crosstalkDelay[leftChannel.crosstalkWritePos] = leftSample;
        rightChannel.crosstalkDelay[rightChannel.crosstalkWritePos] = rightSample;
        
        // Get slightly delayed samples for crosstalk
        int readPos = (leftChannel.crosstalkWritePos + 4) % 8; // ~0.1ms delay at 44.1kHz
        float delayedLeft = leftChannel.crosstalkDelay[readPos];
        float delayedRight = rightChannel.crosstalkDelay[readPos];
        
        // Apply crosstalk bleed
        leftData[sample] = leftSample + bleedAmount * delayedRight;
        rightData[sample] = rightSample + bleedAmount * delayedLeft;
        
        // Advance write positions
        leftChannel.crosstalkWritePos = (leftChannel.crosstalkWritePos + 1) % 8;
        rightChannel.crosstalkWritePos = (rightChannel.crosstalkWritePos + 1) % 8;
    }
}

void Magnetic::updateHeadBumpFilter(ChannelState& channel, float headBumpFreq)
{
    // headBumpFreq comes as 40-120 Hz from parameter
    float frequency = juce::jlimit(40.0f, 120.0f, headBumpFreq);
    
    // Map frequency to gain: higher frequency = more gain
    // 40Hz = 0dB, 120Hz = +12dB - much more obvious  
    float gainDb = (frequency - 40.0f) / (120.0f - 40.0f) * 12.0f;
    gainDb = juce::jlimit(0.0f, 12.0f, gainDb);
    float linearGain = juce::Decibels::decibelsToGain(gainDb);
    
    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, frequency, 0.7f, linearGain);
    
    if (coeffs != nullptr)
        channel.headBumpFilter.coefficients = coeffs;
}

void Magnetic::updateWearFilter(ChannelState& channel, float wearAmount)
{
    // Map wear amount to cutoff frequency: 20kHz (no wear) to 3kHz (max wear) - very dramatic
    float cutoffHz = 20000.0f - wearAmount * 17000.0f;
    cutoffHz = juce::jlimit(3000.0f, 20000.0f, cutoffHz);
    
    // Don't update if cutoff is too close to Nyquist
    if (cutoffHz > sampleRate * 0.45)
        cutoffHz = static_cast<float>(sampleRate * 0.45);
    
    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffHz);
    
    if (coeffs != nullptr)
        channel.wearFilter.coefficients = coeffs;
}

float Magnetic::softClip(float input) noexcept
{
    // Cubic soft clipper for gentle saturation
    if (std::abs(input) <= 1.0f)
        return input - (input * input * input) / 3.0f;
    else
        return input > 0.0f ? 2.0f / 3.0f : -2.0f / 3.0f;
}

}