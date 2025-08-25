#include "Digital.h"
#include "../core/Params.h"
#include "../core/MacroController.h"

namespace ReallyCheap
{

Digital::Digital()
{
    random.setSeedRandomly();
    hostSampleRate = 44100.0;
}

void Digital::prepare(double sampleRate_, int blockSize, int numChannels_)
{
    hostSampleRate = sampleRate_;
    juce::ignoreUnused(blockSize);
    
    // Ensure we have the right number of channels
    channels.resize(numChannels_);
    
    for (auto& channel : channels)
    {
        // Initialize smoothed parameters with longer ramp time to prevent stepping
        const double rampLengthSeconds = 0.05; // 50ms for smoother transitions
        
        channel.smoothedBits.reset(sampleRate_, rampLengthSeconds);
        channel.smoothedSampleRate.reset(sampleRate_, rampLengthSeconds);
        // Initialize to "bypass" values to prevent pop on first enable
        channel.smoothedBits.setCurrentAndTargetValue(0.0f); // 0 mix = bypass
        channel.smoothedSampleRate.setCurrentAndTargetValue(0.0f); // 0 mix = bypass
        
        // Initialize phase accumulator
        channel.phase = 0.0;
        channel.previousInput = 0.0f;
        channel.heldSample = 0.0f;
        channel.lastPhaseIncrement = 0.0f;
        channel.noiseShapingError = 0.0f;
        
        // Initialize frequency-selective state
        channel.highpassState = 0.0f;
        channel.previousInputForHP = 0.0f;
        channel.lowpassState = 0.0f;
        channel.lowpassState2 = 0.0f;
        
        // Initialize filter state
        channel.x1 = channel.x2 = 0.0f;
        channel.y1 = channel.y2 = 0.0f;
    }
    
    reset();
}

void Digital::reset()
{
    for (auto& channel : channels)
    {
        channel.phase = 0.0;
        channel.previousInput = 0.0f;
        channel.heldSample = 0.0f;
        channel.lastPhaseIncrement = 0.0f;
        channel.noiseShapingError = 0.0f;
        channel.highpassState = 0.0f;
        channel.previousInputForHP = 0.0f;
        channel.lowpassState = 0.0f;
        channel.lowpassState2 = 0.0f;
        channel.x1 = channel.x2 = 0.0f;
        channel.y1 = channel.y2 = 0.0f;
        
        // Reset smoothed parameters to bypass state to prevent pops
        channel.smoothedBits.setCurrentAndTargetValue(0.0f);
        channel.smoothedSampleRate.setCurrentAndTargetValue(0.0f);
    }
}

void Digital::process(juce::AudioBuffer<float>& buffer, 
                     juce::AudioPlayHead* playHead, 
                     juce::AudioProcessorValueTreeState& apvts,
                     const MacroController& macro) noexcept
{
    juce::ignoreUnused(playHead);
    
    const int numSamples = buffer.getNumSamples();
    const int bufferChannels = buffer.getNumChannels();
    
    // Get parameters
    const bool digitalOn = *apvts.getRawParameterValue(ParameterIDs::digitalOn) > 0.5f;
    
    if (!digitalOn)
        return;
    
    // Get parameters - now treating them as MIX amounts (0-100%)
    const int baseBitsParam = static_cast<int>(*apvts.getRawParameterValue(ParameterIDs::digitalBits));
    const float baseSRParam = *apvts.getRawParameterValue(ParameterIDs::digitalSR);
    const float jitterAmount = *apvts.getRawParameterValue(ParameterIDs::digitalJitter);
    const bool useAntiAlias = *apvts.getRawParameterValue(ParameterIDs::digitalAA) > 0.5f;
    
    // Convert parameter ranges to mix amounts with smoother scaling curve
    float bitsNormalized = (baseBitsParam - 4.0f) / (16.0f - 4.0f); // 0.0 to 1.0
    float srNormalized = (baseSRParam - 6000.0f) / (44100.0f - 6000.0f); // 0.0 to 1.0
    
    // Apply gentler curves for smoother scaling across full range
    // Use x^1.8 for bits (less aggressive than x^2.5) for more even distribution
    bitsNormalized = std::pow(bitsNormalized, 1.8f);
    srNormalized = std::pow(srNormalized, 1.6f);
    
    // Invert: higher parameter values = LESS effect (more dry)
    const float bitsMixAmount = 1.0f - bitsNormalized; // 16 bits = 0% mix, 4 bits = 100% mix  
    const float srMixAmount = 1.0f - srNormalized; // 44100Hz = 0% mix, 6000Hz = 100% mix
    
    // Apply macro modulation to mix amounts (macro reduces the effect, so reduces mix)
    const float macroSRReduction = (44100.0f - macro.digitalSRFloorHz()) / (44100.0f - 6000.0f);
    const float macroBitsReduction = (16.0f - macro.digitalBitsFloor()) / (16.0f - 4.0f);
    
    const float finalSRMix = juce::jlimit(0.0f, 1.0f, srMixAmount + macroSRReduction);
    const float finalBitsMix = juce::jlimit(0.0f, 1.0f, bitsMixAmount + macroBitsReduction);
    
    // Skip processing entirely if both mixes are near zero (fully dry)
    if (finalSRMix < 0.01f && finalBitsMix < 0.01f)
        return;
    
    // Use much more aggressive sample rate reduction for more obvious effect
    const float fixedSampleRate = 8000.0f; // Lower fixed SR for more dramatic effect
    // Map bit depth with smoother distribution: high mix = low bits (more quantization)
    // Use a gentler curve that spreads the effect more evenly across the range
    const float scaledMix = std::pow(finalBitsMix, 0.8f); // Gentler curve for bit mapping
    juce::ignoreUnused(scaledMix); // Used in individual sample processing
    
    // Update anti-alias filter
    if (useAntiAlias)
    {
        updateBiquadCoeffs(fixedSampleRate * 0.45f);
    }
    
    for (int ch = 0; ch < juce::jmin(bufferChannels, static_cast<int>(channels.size())); ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        auto& channel = channels[ch];
        
        // Update smoothed parameters for mixing
        channel.smoothedBits.setTargetValue(finalBitsMix);
        channel.smoothedSampleRate.setTargetValue(finalSRMix);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float dryInput = channelData[sample];
            float wetOutput = dryInput;
            
            // Get current mix amounts
            const float currentBitsMix = channel.smoothedBits.getNextValue();
            const float currentSRMix = channel.smoothedSampleRate.getNextValue();
            
            // Process with fixed "extreme" settings only if needed
            if (currentSRMix > 0.01f || currentBitsMix > 0.01f)
            {
                // Anti-alias filter
                if (useAntiAlias)
                {
                    wetOutput = processBiquadFilter(channel, wetOutput);
                }
                
                // Sample rate reduction (always apply with fixed rate when mix > 0)
                if (currentSRMix > 0.01f)
                {
                    wetOutput = processSampleRateReduction(channel, wetOutput, fixedSampleRate, jitterAmount);
                }
                
                // Bit depth reduction (apply with variable bits when mix > 0)
                if (currentBitsMix > 0.01f)
                {
                    // Calculate current target bits based on smoothed mix amount
                    const float currentScaledMix = std::pow(currentBitsMix, 0.8f);
                    const float currentTargetBits = 16.0f - (currentScaledMix * 12.0f);
                    wetOutput = processHardQuantization(channel, wetOutput, currentTargetBits);
                }
            }
            
            // Always apply consistent wet/dry mixing - don't skip SRR based on bitcrush level
            float finalOutput = dryInput;
            
            // Apply sample rate reduction mix first (always if enabled)
            if (currentSRMix > 0.01f)
            {
                // Create SRR-only processed signal for clean mixing
                float srrOnlyOutput = dryInput;
                srrOnlyOutput = processSampleRateReduction(channel, srrOnlyOutput, fixedSampleRate, jitterAmount);
                
                // Mix dry input with SRR-only output
                finalOutput = dryInput * (1.0f - currentSRMix) + srrOnlyOutput * currentSRMix;
            }
            
            // Apply bit depth reduction mix (independently of SR processing)
            if (currentBitsMix > 0.01f)
            {
                // Always process bit reduction on the current signal state
                const float currentScaledMix = std::pow(currentBitsMix, 0.8f);
                const float currentTargetBits = 16.0f - (currentScaledMix * 12.0f);
                float bitProcessedOutput = processHardQuantization(channel, finalOutput, currentTargetBits);
                
                // Mix current signal with bit-processed signal
                finalOutput = finalOutput * (1.0f - currentBitsMix) + bitProcessedOutput * currentBitsMix;
            }
            
            channelData[sample] = finalOutput;
        }
    }
}

float Digital::processBiquadFilter(ChannelState& channel, float input) noexcept
{
    // Transposed Direct Form II biquad filter
    float output = currentCoeffs.b0 * input + channel.x1;
    channel.x1 = currentCoeffs.b1 * input - currentCoeffs.a1 * output + channel.x2;
    channel.x2 = currentCoeffs.b2 * input - currentCoeffs.a2 * output;
    
    return output;
}

float Digital::processSampleRateReduction(ChannelState& channel, float input, 
                                        float targetSR, float jitterAmount) noexcept
{
    // More aggressive phase accumulator with enhanced aliasing
    double phaseIncrement = targetSR / hostSampleRate;
    
    // Apply jitter if requested - make it more pronounced
    if (jitterAmount > 0.0f)
    {
        float jitter = generateJitterOffset(jitterAmount);
        phaseIncrement *= (1.0 + jitter * 2.0f); // Double jitter effect
        phaseIncrement = juce::jmax(phaseIncrement, 0.0001);
    }
    
    // Allow more extreme phase accumulation for more obvious effect
    phaseIncrement = juce::jmin(phaseIncrement, 0.95); // Allow faster accumulation
    
    // Reduce smoothing to allow more stepping artifacts (more character)
    float currentPhaseIncrement = static_cast<float>(phaseIncrement);
    const float smoothingFactor = 0.85f; // Less smoothing for more character
    channel.lastPhaseIncrement = channel.lastPhaseIncrement * smoothingFactor + 
                                currentPhaseIncrement * (1.0f - smoothingFactor);
    
    // Use smoothed phase increment
    phaseIncrement = static_cast<double>(channel.lastPhaseIncrement);
    
    channel.phase += phaseIncrement;
    
    if (channel.phase >= 1.0)
    {
        // Calculate interpolation factor for sub-sample accuracy
        double overshoot = channel.phase - 1.0;
        double interpFactor = overshoot / phaseIncrement;
        
        // Use less interpolation for more aliasing and digital character
        float interpolatedSample = channel.previousInput + 
                                 (input - channel.previousInput) * (1.0f - static_cast<float>(interpFactor)) * 0.7f;
        
        // Add some of the raw input for more aliasing
        interpolatedSample += input * 0.3f;
        
        channel.heldSample = interpolatedSample;
        channel.phase = std::fmod(channel.phase, 1.0);
    }
    
    channel.previousInput = input;
    return channel.heldSample;
}

float Digital::processBitDepthReduction(ChannelState& channel, float input, float bits) noexcept
{
    // Mid-tread quantizer with proper step size
    const float stepSize = 2.0f / (std::pow(2.0f, bits) - 1.0f);
    
    // Add TPDF dither
    float dither = generateTPDFDither(stepSize);
    float dithered = input + dither;
    
    // Quantize
    float quantized = std::round(dithered / stepSize) * stepSize;
    
    // Apply 1st-order noise shaping for low bit depths
    if (bits <= 8.0f)
    {
        float error = quantized - input;
        quantized -= channel.noiseShapingError;
        channel.noiseShapingError = error * 0.5f; // 1st order coefficient
    }
    
    // Clamp to valid range
    return juce::jlimit(-1.0f, 1.0f, quantized);
}

float Digital::processHardQuantization(ChannelState& channel, float input, float bits) noexcept
{
    // Enhanced frequency-selective bitcrushing - heavily target low frequencies
    
    // Use even lower crossover to focus more on bass/low-mids
    const float crossoverFreq = 600.0f; // Hz - lower to target more of the low end
    const float rc = 1.0f / (2.0f * juce::MathConstants<float>::pi * crossoverFreq);
    const float dt = 1.0f / static_cast<float>(hostSampleRate);
    const float alpha = rc / (rc + dt);
    
    // Apply 1-pole highpass filter
    float highFreqs = alpha * (channel.highpassState + input - channel.previousInputForHP);
    channel.highpassState = highFreqs;
    channel.previousInputForHP = input;
    
    // Low frequencies = total - high frequencies  
    float lowFreqs = input - highFreqs;
    
    // Apply additional low-end boost before quantization for more aggressive targeting
    lowFreqs *= 1.8f; // Increased boost to emphasize low frequency content
    
    // Apply EXTREMELY aggressive quantization to low frequencies only
    float scaledLows = lowFreqs * 2.5f; // Drive even harder for more low-end destruction
    
    // Hard quantization with no dithering for maximum gnarl on lows only
    const float levels = std::pow(2.0f, bits) - 1.0f;
    const float stepSize = 2.0f / levels;
    
    // Aggressive quantization with floor/ceiling for harsh stepping on lows
    float quantizedLows;
    if (scaledLows >= 0.0f)
        quantizedLows = std::floor(scaledLows / stepSize + 0.5f) * stepSize;
    else
        quantizedLows = std::ceil(scaledLows / stepSize - 0.5f) * stepSize;
    
    // Apply aliasing and distortion for extra digital nastiness
    if (bits <= 10.0f)
    {
        // Cubic distortion for harmonic generation
        float distortion = quantizedLows * quantizedLows * quantizedLows * 0.4f;
        quantizedLows += distortion * (1.0f - bits / 10.0f);
        
        // Add some intermodulation at very low bit depths
        if (bits <= 6.0f)
        {
            float imd = std::sin(quantizedLows * 8.0f) * 0.15f * (1.0f - bits / 6.0f);
            quantizedLows += imd;
        }
    }
    
    // Saturate to control levels but keep the gnarl
    quantizedLows = std::tanh(quantizedLows * 0.7f) / 0.7f;
    
    // Add even more low-end emphasis to the crushed signal
    quantizedLows *= 1.5f; // +3.5dB boost for more low-end presence
    
    // Hi-damping filter after bit reduction - gentle high-frequency rolloff
    const float hiDampCutoff = 8000.0f; // Hz - rolloff highs for analog warmth
    const float hiDampRc = 1.0f / (2.0f * juce::MathConstants<float>::pi * hiDampCutoff);
    const float hiDampAlpha = dt / (hiDampRc + dt);
    
    channel.lowpassState2 = channel.lowpassState2 + hiDampAlpha * (quantizedLows - channel.lowpassState2);
    quantizedLows = quantizedLows * 0.7f + channel.lowpassState2 * 0.3f; // Apply hi-damping
    
    // Only apply additional smoothing at extremely low bit depths
    if (bits <= 5.0f)
    {
        const float lpCutoff = 3500.0f; // Lower cutoff for more analog feel
        const float lpRc = 1.0f / (2.0f * juce::MathConstants<float>::pi * lpCutoff);
        const float lpAlpha = dt / (lpRc + dt);
        
        channel.lowpassState = channel.lowpassState + lpAlpha * (quantizedLows - channel.lowpassState);
        quantizedLows = quantizedLows * 0.75f + channel.lowpassState * 0.25f; // More filtering at extreme settings
    }
    
    quantizedLows = juce::jlimit(-1.5f, 1.5f, quantizedLows);
    
    // Combine: clean highs + heavily crushed and hi-damped lows
    float output = highFreqs + quantizedLows;
    
    return juce::jlimit(-1.3f, 1.3f, output);
}

void Digital::updateBiquadCoeffs(float cutoffFreq) noexcept
{
    // Butterworth lowpass filter coefficients
    const float normalizedFreq = juce::jlimit(0.001f, 0.499f, cutoffFreq / static_cast<float>(hostSampleRate));
    const float omega = 2.0f * juce::MathConstants<float>::pi * normalizedFreq;
    const float cos_omega = std::cos(omega);
    const float sin_omega = std::sin(omega);
    const float Q = 0.707f; // Butterworth Q
    const float alpha = sin_omega / (2.0f * Q);
    
    const float a0 = 1.0f + alpha;
    currentCoeffs.b0 = (1.0f - cos_omega) / (2.0f * a0);
    currentCoeffs.b1 = (1.0f - cos_omega) / a0;
    currentCoeffs.b2 = currentCoeffs.b0;
    currentCoeffs.a1 = -2.0f * cos_omega / a0;
    currentCoeffs.a2 = (1.0f - alpha) / a0;
}

float Digital::generateTPDFDither(float stepSize) noexcept
{
    // Triangular Probability Density Function dither (sum of two uniform distributions)
    float uniform1 = (random.nextFloat() - 0.5f) * stepSize;
    float uniform2 = (random.nextFloat() - 0.5f) * stepSize;
    return (uniform1 + uniform2) * 0.5f;
}

float Digital::generateJitterOffset(float amount) noexcept
{
    // Zero-mean jitter for clock instability
    float jitter = (random.nextFloat() - 0.5f) * 2.0f * amount;
    return juce::jlimit(-0.5f, 0.5f, jitter * 0.1f);
}

// Legacy compatibility methods (minimal implementations)
float Digital::generateDither() noexcept 
{ 
    return generateTPDFDither(2.0f / 255.0f); 
}

float Digital::quantizeToBits(float input, int bits, float) noexcept 
{ 
    ChannelState tempChannel;
    tempChannel.noiseShapingError = 0.0f;
    return processBitDepthReduction(tempChannel, input, static_cast<float>(bits));
}

float Digital::processAntiAlias(ChannelState& channel, float input, float cutoffFreq) noexcept 
{ 
    updateBiquadCoeffs(cutoffFreq);
    return processBiquadFilter(channel, input);
}

float Digital::generateJitter(ChannelState&, float amount) noexcept 
{ 
    return generateJitterOffset(amount); 
}

float Digital::processSimpleAntiAlias(ChannelState& channel, float input, float cutoffFreq) noexcept 
{ 
    return processAntiAlias(channel, input, cutoffFreq); 
}

float Digital::generateSimpleJitter(ChannelState& channel, float amount) noexcept 
{ 
    return generateJitter(channel, amount); 
}

float Digital::generateSimpleDither() noexcept 
{ 
    return generateDither(); 
}

float Digital::quantizeWithDither(float input, int bits, float) noexcept 
{ 
    return quantizeToBits(input, bits, 0.0f); 
}

float Digital::quantizeHard(float input, int bits) noexcept 
{ 
    const float stepSize = 2.0f / (std::pow(2.0f, bits) - 1.0f);
    float quantized = std::round(input / stepSize) * stepSize;
    return juce::jlimit(-1.0f, 1.0f, quantized);
}

float Digital::processSampleRateReduction(ChannelState& channel, float input, float targetSR, bool, float jitterAmount) noexcept 
{ 
    return processSampleRateReduction(channel, input, targetSR, jitterAmount); 
}

float Digital::processBitDepthReduction(ChannelState& channel, float input, int targetBits) noexcept 
{ 
    return processBitDepthReduction(channel, input, static_cast<float>(targetBits)); 
}

float Digital::processAntiAliasingFilter(ChannelState& channel, float input, float cutoffFreq) noexcept 
{ 
    return processAntiAlias(channel, input, cutoffFreq); 
}

}