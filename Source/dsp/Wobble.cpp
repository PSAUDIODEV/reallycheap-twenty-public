#include "Wobble.h"
#include "../core/Params.h"
#include "../core/MacroController.h"
#include <cmath>

namespace ReallyCheap
{

Wobble::Wobble()
{
    random.setSeedRandomly();
}

void Wobble::prepare(double sampleRate_, int blockSize, int numChannels_)
{
    juce::ignoreUnused(blockSize);
    sampleRate = sampleRate_;
    numChannels = numChannels_;
    
    channels.clear();
    channels.resize(numChannels);
    
    // Smaller delay buffer for subtle tape modulation
    // Based on research: 30-100ms typical for pitch shifting without echo artifacts
    const float maxDelayMs = 50.0f; // Middle ground for quality
    int maxDelayInSamples = static_cast<int>(std::ceil(maxDelayMs * 0.001 * sampleRate));
    
    for (auto& channel : channels)
    {
        // Initialize circular buffer for variable delay
        channel.delaySize = maxDelayInSamples;
        channel.delayLine.resize(channel.delaySize);
        std::fill(channel.delayLine.begin(), channel.delayLine.end(), 0.0f);
        channel.delayWritePos = 0;
        
        // Initialize modulation state
        channel.lfoPhase = 0.0;
        channel.prevModValue = 0.0f;
        channel.jitterSmooth = 0.0f;
        
        // Initialize crossfade state for smooth transitions
        channel.crossfadeAmount = 0.0f;
        channel.oldReadPos = 0.0f;
        channel.newReadPos = 0.0f;
        
        // Anti-aliasing filter state (2nd order Butterworth)
        channel.lpf_x1 = 0.0f;
        channel.lpf_x2 = 0.0f;
        channel.lpf_y1 = 0.0f;
        channel.lpf_y2 = 0.0f;
    }
    
    reset();
}

void Wobble::reset()
{
    for (int ch = 0; ch < channels.size(); ++ch)
    {
        auto& channel = channels[ch];
        
        // Reset phases
        channel.lfoPhase = (ch == 1) ? 0.25 : 0.0; // 90° offset for stereo
        channel.prevModValue = 0.0f;
        channel.jitterSmooth = 0.0f;
        
        // Clear delay buffer
        std::fill(channel.delayLine.begin(), channel.delayLine.end(), 0.0f);
        channel.delayWritePos = 0;
        
        // Reset filter states
        channel.lpf_x1 = channel.lpf_x2 = 0.0f;
        channel.lpf_y1 = channel.lpf_y2 = 0.0f;
        
        // Reset crossfade
        channel.crossfadeAmount = 0.0f;
        channel.oldReadPos = 0.0f;
        channel.newReadPos = 0.0f;
    }
}

void Wobble::process(juce::AudioBuffer<float>& buffer, 
                     juce::AudioPlayHead* playHead, 
                     juce::AudioProcessorValueTreeState& apvts,
                     const MacroController& macro) noexcept
{
    const int numSamples = buffer.getNumSamples();
    const int bufferChannels = buffer.getNumChannels();
    
    // Get parameters
    const bool wobbleOn = *apvts.getRawParameterValue(ParameterIDs::wobbleOn) > 0.5f;
    if (!wobbleOn) return;
    
    const float baseDepth = *apvts.getRawParameterValue(ParameterIDs::wobbleDepth);
    const float baseRateHz = *apvts.getRawParameterValue(ParameterIDs::wobbleRateHz);
    const bool monoMode = *apvts.getRawParameterValue(ParameterIDs::wobbleMono) > 0.5f;
    const float flutter = *apvts.getRawParameterValue(ParameterIDs::wobbleFlutter);
    const float drift = *apvts.getRawParameterValue(ParameterIDs::wobbleDrift);
    const float jitter = *apvts.getRawParameterValue(ParameterIDs::wobbleJitter);
    const float stereoLink = *apvts.getRawParameterValue(ParameterIDs::wobbleStereoLink);
    
    // Apply macro modulation
    const float depthGain = macro.wobbleDepthGain();
    const float depth = baseDepth * depthGain;
    const float rateHz = juce::jlimit(0.1f, 10.0f, baseRateHz);
    
    // Calculate modulation parameters based on research
    // Key insight: Variable sampling rate approach is smoother than position modulation
    // We'll simulate this by using smooth delay changes
    
    // Process each channel
    for (int ch = 0; ch < std::min(bufferChannels, static_cast<int>(channels.size())); ++ch)
    {
        auto& channel = channels[ch];
        auto* channelData = buffer.getWritePointer(ch);
        
        // Calculate phase increment
        const double phaseInc = static_cast<double>(rateHz) / sampleRate;
        
        // Anti-aliasing filter coefficients (Butterworth at 15kHz)
        const float cutoff = 15000.0f / static_cast<float>(sampleRate);
        const float c = 1.0f / std::tan(juce::MathConstants<float>::pi * cutoff);
        const float c2 = c * c;
        const float sqrt2c = std::sqrt(2.0f) * c;
        const float a0 = c2 + sqrt2c + 1.0f;
        const float a1 = 2.0f * (1.0f - c2) / a0;
        const float a2 = (c2 - sqrt2c + 1.0f) / a0;
        const float b0 = 1.0f / a0;
        const float b1 = 2.0f / a0;
        const float b2 = 1.0f / a0;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Apply anti-aliasing filter to input
            float input = channelData[sample];
            float filtered = b0 * input + b1 * channel.lpf_x1 + b2 * channel.lpf_x2
                           - a1 * channel.lpf_y1 - a2 * channel.lpf_y2;
            
            channel.lpf_x2 = channel.lpf_x1;
            channel.lpf_x1 = input;
            channel.lpf_y2 = channel.lpf_y1;
            channel.lpf_y1 = filtered;
            
            // Store filtered input in delay line
            channel.delayLine[channel.delayWritePos] = filtered;
            
            // Generate modulation signals
            float wowValue = std::sin(channel.lfoPhase * juce::MathConstants<float>::twoPi);
            
            // Flutter: Higher frequency, smaller amplitude
            float flutterPhase = channel.lfoPhase * 7.0f; // 7x main rate
            float flutterValue = std::sin(flutterPhase * juce::MathConstants<float>::twoPi);
            
            // Drift: Very slow quasi-random modulation
            float driftPhase = channel.lfoPhase * 0.03f; // Much slower
            float driftValue = std::sin(driftPhase * juce::MathConstants<float>::twoPi * 1.414f); // Irrational multiplier
            
            // Jitter: Filtered noise
            float targetJitter = (random.nextFloat() - 0.5f) * 2.0f;
            channel.jitterSmooth = channel.jitterSmooth * 0.98f + targetJitter * 0.02f; // Heavy filtering
            
            // Combine modulation sources with proper scaling
            // Research shows typical wow/flutter is 0.08% to 0.5% speed variation
            // For a 50ms buffer, this translates to 0.04ms to 0.25ms delay variation
            
            float totalMod = wowValue * depth * 0.7f +           // Main wow component
                           flutterValue * flutter * 0.15f +      // Flutter is subtle
                           driftValue * drift * 0.5f +           // Drift is noticeable but clean
                           channel.jitterSmooth * jitter * 0.3f; // Jitter is audible but clean
            
            // Apply stereo processing
            if (ch == 1 && !monoMode)
            {
                // Independent stereo with optional linking
                float offsetPhase = channel.lfoPhase + 0.25; // 90° offset
                if (offsetPhase >= 1.0) offsetPhase -= 1.0;
                float stereoWow = std::sin(offsetPhase * juce::MathConstants<float>::twoPi);
                
                float independentMod = stereoWow * depth * 0.7f +
                                     flutterValue * flutter * 0.15f +
                                     driftValue * drift * 0.5f +
                                     channel.jitterSmooth * jitter * 0.3f;
                
                // Blend with left channel's modulation based on link amount
                if (stereoLink > 0.0f && channels.size() > 0)
                {
                    totalMod = independentMod * (1.0f - stereoLink) + channels[0].prevModValue * stereoLink;
                }
                else
                {
                    totalMod = independentMod;
                }
            }
            else if (ch == 1 && monoMode && channels.size() > 0)
            {
                // Use left channel's modulation
                totalMod = channels[0].prevModValue;
            }
            
            // Smooth modulation to prevent zipper noise
            float smoothedMod = channel.prevModValue * 0.9f + totalMod * 0.1f;
            channel.prevModValue = smoothedMod;
            
            // Calculate delay in samples
            // Research suggests 0.5-2ms variation for subtle effect, up to 10ms for extreme
            float delayVariationMs = smoothedMod * 2.0f; // ±2ms variation at full depth
            float delaySamples = std::abs(delayVariationMs * 0.001f * static_cast<float>(sampleRate));
            
            // Add a base delay to ensure we're always reading from the past
            float baseDelaySamples = 10.0f; // 10 sample base delay
            float totalDelaySamples = baseDelaySamples + delaySamples;
            
            // Calculate read position
            float readPos = static_cast<float>(channel.delayWritePos) - totalDelaySamples;
            while (readPos < 0.0f) readPos += static_cast<float>(channel.delaySize);
            
            // Hermite interpolation for smooth pitch shifting
            int idx0 = static_cast<int>(readPos);
            float fraction = readPos - static_cast<float>(idx0);
            
            // Get 4 points for Hermite interpolation
            int idx_m1 = (idx0 - 1 + channel.delaySize) % channel.delaySize;
            int idx_p1 = (idx0 + 1) % channel.delaySize;
            int idx_p2 = (idx0 + 2) % channel.delaySize;
            
            float y_m1 = channel.delayLine[idx_m1];
            float y0 = channel.delayLine[idx0];
            float y1 = channel.delayLine[idx_p1];
            float y2 = channel.delayLine[idx_p2];
            
            // Hermite interpolation coefficients
            float c0 = y0;
            float c1 = 0.5f * (y1 - y_m1);
            float c2 = y_m1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
            float c3 = 0.5f * (y2 - y_m1) + 1.5f * (y0 - y1);
            
            // Calculate interpolated output
            float output = ((c3 * fraction + c2) * fraction + c1) * fraction + c0;
            
            // Mix with dry signal based on depth (subtle blending)
            float wetMix = juce::jlimit(0.0f, 1.0f, depth * 2.0f); // Full wet at 50% depth
            channelData[sample] = output * wetMix + channelData[sample] * (1.0f - wetMix);
            
            // Advance write position
            channel.delayWritePos = (channel.delayWritePos + 1) % channel.delaySize;
            
            // Advance LFO phase
            channel.lfoPhase += phaseInc;
            if (channel.lfoPhase >= 1.0)
                channel.lfoPhase -= 1.0;
        }
    }
}

float Wobble::calculateLfoValue(ChannelState& channel, bool useSync, float rateHz) noexcept
{
    // Not used in new implementation
    return 0.0f;
}

void Wobble::updateDrift(ChannelState& channel, float driftAmount) noexcept
{
    // Not used in new implementation  
}

float Wobble::generateJitter(ChannelState& channel, float jitterAmount) noexcept
{
    // Not used in new implementation
    return 0.0f;
}

void Wobble::updateTempo(juce::AudioPlayHead* playHead) noexcept
{
    // Not used in new implementation
}

double Wobble::getSyncedFrequency(float rateHz, double bpm) const noexcept
{
    // Not used in new implementation
    return 0.0;
}

}