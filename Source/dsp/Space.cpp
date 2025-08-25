#include "Space.h"
#include "../core/Params.h"
#include "../core/MacroController.h"

namespace ReallyCheap
{

Space::Space()
{
}

void Space::prepare(double sampleRate_, int samplesPerBlock, int numChannels_)
{
    sampleRate = sampleRate_;
    blockSize = samplesPerBlock;
    numChannels = numChannels_;
    
    // Setup pre-delay lines (max 30ms)
    preDelayLines.clear();
    preDelayLines.resize(numChannels);
    for (auto& delayLine : preDelayLines)
    {
        delayLine.prepare(sampleRate, 30); // 30ms max
    }
    
    // Setup tone controls
    tiltEQs.clear();
    tiltEQs.resize(numChannels);
    for (auto& eq : tiltEQs)
    {
        eq.prepare(sampleRate);
    }
    
    // Setup algorithmic reverb - juicy multi-tap delay network
    reverbDelays.clear();
    reverbDelays.resize(numChannels);
    for (auto& channelDelays : reverbDelays)
    {
        // Different delay times for each channel and tap - extended range for longer tails
        std::vector<int> delayTimesMs = {41, 67, 103, 139, 191, 229, 283, 337, 389, 443, 509, 571}; // More prime delays for complexity
        channelDelays.resize(delayTimesMs.size());
        
        for (size_t i = 0; i < channelDelays.size(); ++i)
        {
            int delaySamples = static_cast<int>(delayTimesMs[i] * 0.001 * sampleRate);
            channelDelays[i].resize(delaySamples, 0.0f);
        }
    }
    
    // Initialize reverb state
    reverbState.clear();
    reverbState.resize(numChannels);
    
    // Setup parameter smoothing
    const double smoothTime = 0.02; // 20ms
    mixSmoothed.reset(sampleRate, smoothTime);
    preDelaySmoothed.reset(sampleRate, smoothTime);
    toneSmoothed.reset(sampleRate, smoothTime * 2);
    reverbTimeSmoothed.reset(sampleRate, smoothTime * 4); // Slower for reverb time
    roomSizeSmoothed.reset(sampleRate, smoothTime * 4);
    
    // Initialize temp buffers
    wetBuffer.setSize(numChannels, samplesPerBlock);
    dryBuffer.setSize(numChannels, samplesPerBlock);
    
    reset();
}

void Space::reset()
{
    for (auto& delayLine : preDelayLines)
        delayLine.reset();
    
    for (auto& eq : tiltEQs)
        eq.reset();
    
    // Clear reverb delays
    for (auto& channelDelays : reverbDelays)
    {
        for (auto& delay : channelDelays)
        {
            std::fill(delay.begin(), delay.end(), 0.0f);
        }
    }
    
    // Reset reverb state
    for (auto& state : reverbState)
    {
        state.writePos = 0;
        state.feedback = 0.6f;
        state.diffusion = 0.5f;
        state.lowpass1 = 0.0f;
        state.lowpass2 = 0.0f;
        state.allpass1 = 0.0f;
        state.allpass2 = 0.0f;
    }
    
    mixSmoothed.setCurrentAndTargetValue(ParameterDefaults::spaceMix);
    preDelaySmoothed.setCurrentAndTargetValue(ParameterDefaults::spacePreDelayMs);
    toneSmoothed.setCurrentAndTargetValue(ParameterDefaults::spaceTone);
    reverbTimeSmoothed.setCurrentAndTargetValue(0.8f);
    roomSizeSmoothed.setCurrentAndTargetValue(0.6f);
}

void Space::process(juce::AudioBuffer<float>& buffer,
                   juce::AudioPlayHead* playHead,
                   juce::AudioProcessorValueTreeState& apvts,
                   const MacroController& macro) noexcept
{
    juce::ignoreUnused(playHead);
    
    const int numSamples = buffer.getNumSamples();
    const int bufferChannels = buffer.getNumChannels();
    
    // Get parameters
    const bool spaceOn = *apvts.getRawParameterValue(ParameterIDs::spaceOn) > 0.5f;
    
    // Debug output for first few calls
    static int debugCallCount = 0;
    if (debugCallCount < 10)
    {
        DBG("Space::process - Call " << debugCallCount << ", spaceOn: " << (spaceOn ? "YES" : "NO") << ", numSamples: " << numSamples);
        debugCallCount++;
    }
    
    if (!spaceOn)
    {
        return;
    }
    
    const float baseMix = *apvts.getRawParameterValue(ParameterIDs::spaceMix);
    const float time = *apvts.getRawParameterValue(ParameterIDs::spaceTime);
    const float tone = *apvts.getRawParameterValue(ParameterIDs::spaceTone);
    const float preDelayMs = *apvts.getRawParameterValue(ParameterIDs::spacePreDelayMs);
    
    // Apply macro modulation with guardrails - use more generous cap
    const float mix = juce::jmin(baseMix, juce::jmax(0.25f, macro.spaceMixCap())); // At least 25% mix allowed
    
    // Debug processing parameters
    static int processDebugCount = 0;
    if (processDebugCount < 5)
    {
        DBG("Space::process - PROCESSING! baseMix: " << baseMix << ", mixCap: " << macro.spaceMixCap() << ", finalMix: " << mix << ", time: " << time);
        juce::ignoreUnused(macro); // Remove warning since we still reference it
        processDebugCount++;
    }
    
    // Map time parameter to reverb characteristics - much longer tails
    const float reverbTime = 1.2f + time * 4.8f; // 1.2s to 6.0s decay time
    const float roomSize = 0.2f + time * 0.6f;   // 0.2 to 0.8 room size (smaller rooms = less damping)
    
    // Update smoothed parameters
    mixSmoothed.setTargetValue(mix);
    preDelaySmoothed.setTargetValue(preDelayMs);
    toneSmoothed.setTargetValue(tone);
    reverbTimeSmoothed.setTargetValue(reverbTime);
    roomSizeSmoothed.setTargetValue(roomSize);
    
    // Store dry signal for mix
    dryBuffer.makeCopyOf(buffer);
    
    // Process wet signal
    wetBuffer.makeCopyOf(buffer);
    
    // Apply pre-delay
    const float currentPreDelay = preDelaySmoothed.getCurrentValue();
    const float preDelaySamples = currentPreDelay * 0.001f * static_cast<float>(sampleRate);
    
    for (int ch = 0; ch < std::min(bufferChannels, static_cast<int>(preDelayLines.size())); ++ch)
    {
        auto* wetData = wetBuffer.getWritePointer(ch);
        auto& delayLine = preDelayLines[ch];
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            wetData[sample] = delayLine.processSample(wetData[sample], preDelaySamples);
        }
    }
    
    // Apply algorithmic reverb processing
    processAlgorithmicReverb(wetBuffer);
    
    // Apply tone control
    const float currentTone = toneSmoothed.getCurrentValue();
    
    for (int ch = 0; ch < std::min(bufferChannels, static_cast<int>(tiltEQs.size())); ++ch)
    {
        auto& eq = tiltEQs[ch];
        auto* wetData = wetBuffer.getWritePointer(ch);
        
        // Update filter coefficients once per block
        eq.updateCoeffs(currentTone, sampleRate);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            wetData[sample] = eq.processSample(wetData[sample]);
        }
    }
    
    // Mix wet and dry signals
    for (int ch = 0; ch < bufferChannels; ++ch)
    {
        auto* outputData = buffer.getWritePointer(ch);
        auto* wetData = wetBuffer.getReadPointer(ch);
        auto* dryData = dryBuffer.getReadPointer(ch);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float currentMix = mixSmoothed.getNextValue();
            outputData[sample] = dryData[sample] * (1.0f - currentMix) + wetData[sample] * currentMix;
            
            // Safety check
            if (!std::isfinite(outputData[sample]))
                outputData[sample] = 0.0f;
        }
    }
}

void Space::processAlgorithmicReverb(juce::AudioBuffer<float>& buffer) noexcept
{
    const int numSamples = buffer.getNumSamples();
    const int bufferChannels = buffer.getNumChannels();
    
    const float currentReverbTime = reverbTimeSmoothed.getCurrentValue();
    const float currentRoomSize = roomSizeSmoothed.getCurrentValue();
    
    // Calculate feedback based on reverb time - SAFE feedback levels to prevent runaway
    const float targetFeedback = 0.4f + currentReverbTime * 0.25f; // 0.4 to 0.65 feedback (SAFER)
    const float diffusion = 0.6f + currentRoomSize * 0.2f;         // 0.6 to 0.8 diffusion (SAFER)
    
    for (int ch = 0; ch < std::min(bufferChannels, static_cast<int>(reverbDelays.size())); ++ch)
    {
        auto& channelDelays = reverbDelays[ch];
        auto& state = reverbState[ch];
        auto* data = buffer.getWritePointer(ch);
        
        // Smooth feedback changes - allow higher feedback with stability
        state.feedback += (targetFeedback - state.feedback) * 0.0005f; // Slower changes for stability at high feedback
        state.diffusion += (diffusion - state.diffusion) * 0.0005f;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = data[sample];
            float output = 0.0f;
            
            // Multi-tap delay network with feedback and diffusion
            for (size_t i = 0; i < channelDelays.size(); ++i)
            {
                auto& delay = channelDelays[i];
                const int delayLength = static_cast<int>(delay.size());
                
                // Read from delay line
                float delayedSample = delay[state.writePos % delayLength];
                
                // Apply different gains for each tap to create complexity
                float tapGain = 0.8f / static_cast<float>(channelDelays.size());
                if (i % 2 == 0) tapGain *= 1.2f; // Emphasize even taps slightly
                
                output += delayedSample * tapGain;
                
                // Write new sample to delay line with feedback
                float inputWithFeedback = input + delayedSample * state.feedback;
                
                // Apply moderate diffusion for spaciousness without killing the tail
                if (i < 4) // Apply diffusion to more taps for better reverb density
                {
                    float allpassOut = inputWithFeedback + state.allpass1 * state.diffusion * 0.7f; // Moderate diffusion
                    state.allpass1 = inputWithFeedback - allpassOut * state.diffusion * 0.7f;
                    inputWithFeedback = allpassOut;
                    
                    // Second diffusion stage for first few taps
                    if (i < 2)
                    {
                        allpassOut = inputWithFeedback + state.allpass2 * state.diffusion * 0.5f;
                        state.allpass2 = inputWithFeedback - allpassOut * state.diffusion * 0.5f;
                        inputWithFeedback = allpassOut;
                    }
                }
                
                delay[state.writePos % delayLength] = inputWithFeedback;
            }
            
            // Light high-frequency damping for natural air absorption - but not too much
            const float dampening = 0.92f + currentRoomSize * 0.06f; // 0.92 to 0.98 - moderate damping for tail preservation
            state.lowpass1 = state.lowpass1 * dampening + output * (1.0f - dampening);
            state.lowpass2 = state.lowpass2 * 0.88f + state.lowpass1 * 0.12f; // More gentle but present second stage
            
            // Add some stereo widening by inverting phase on one channel
            if (ch == 1)
                state.lowpass2 *= -0.8f;
            
            // SAFETY LIMITING - prevent reverb feedback runaway
            state.lowpass2 = juce::jlimit(-1.5f, 1.5f, state.lowpass2);
            if (!std::isfinite(state.lowpass2))
                state.lowpass2 = 0.0f;
            
            data[sample] = state.lowpass2;
            
            state.writePos++;
        }
    }
}

int Space::getLatencySamples() const noexcept
{
    return 0; // Algorithmic reverb has no latency
}

void Space::requestIRPreload(const juce::File& folder)
{
    juce::ignoreUnused(folder); // No longer needed for algorithmic reverb
}

//==============================================================================
// Helper Classes Implementation
//==============================================================================

void Space::PreDelayLine::prepare(double sampleRate, int maxDelayMs)
{
    maxDelaySamples = static_cast<int>(maxDelayMs * 0.001 * sampleRate);
    buffer.resize(maxDelaySamples + 1, 0.0f);
    writePos = 0;
}

void Space::PreDelayLine::reset()
{
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    writePos = 0;
}

float Space::PreDelayLine::processSample(float input, float delaySamples) noexcept
{
    // Clamp delay to valid range
    delaySamples = juce::jlimit(0.0f, static_cast<float>(maxDelaySamples), delaySamples);
    
    // Write input to buffer
    buffer[writePos] = input;
    
    // Calculate read position with fractional delay
    float readPos = static_cast<float>(writePos) - delaySamples;
    if (readPos < 0.0f)
        readPos += static_cast<float>(buffer.size());
    
    // Linear interpolation
    int readIndex1 = static_cast<int>(readPos);
    int readIndex2 = (readIndex1 + 1) % static_cast<int>(buffer.size());
    float fraction = readPos - static_cast<float>(readIndex1);
    
    float output = buffer[readIndex1] + fraction * (buffer[readIndex2] - buffer[readIndex1]);
    
    // Advance write position
    writePos = (writePos + 1) % static_cast<int>(buffer.size());
    
    return output;
}

void Space::TiltEQ::prepare(double sampleRate)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 1;
    spec.numChannels = 1;
    
    lowShelf.prepare(spec);
    highShelf.prepare(spec);
    
    reset();
}

void Space::TiltEQ::updateCoeffs(float tiltAmount, double sampleRate)
{
    // Map tilt amount (-1 to +1) to shelf gains
    // Negative = darker (boost low, cut high)
    // Positive = brighter (cut low, boost high)
    
    float lowGainDb = -tiltAmount * 2.0f;  // ±2dB at 200Hz - less low cut
    float highGainDb = tiltAmount * 8.0f;  // ±8dB at 4kHz - even more high-end boost available
    
    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, 200.0f, 0.707f, juce::Decibels::decibelsToGain(lowGainDb));
    lowShelf.coefficients = lowCoeffs;
    
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, 4000.0f, 0.707f, juce::Decibels::decibelsToGain(highGainDb));
    highShelf.coefficients = highCoeffs;
}

float Space::TiltEQ::processSample(float input) noexcept
{
    return highShelf.processSample(lowShelf.processSample(input));
}

void Space::TiltEQ::reset()
{
    lowShelf.reset();
    highShelf.reset();
}

}