#pragma once

#include <JuceHeader.h>
#include "../core/Params.h"

namespace ReallyCheap
{

// Forward declaration
class MacroController;

/**
 * Digital Module - Virtual ADC Model
 * 
 * BRR (Bit Rate Reduction): Mid-tread quantizer with TPDF dither
 * - Step size Δ = 2 / (2^bits - 1)
 * - TPDF dither: sum of two independent uniform distributions
 * - Optional 1st-order noise shaping for bits ≤ 8
 * 
 * SRR (Sample Rate Reduction): Phase-accumulator strobe with linear interpolation
 * - Phase accumulates at targetSR / hostSR rate
 * - Strobes (samples) when phase ≥ 1
 * - Linear interpolation for sub-sample accuracy
 * - Jitter modulates phase increment
 * 
 * Anti-alias: Biquad lowpass at 0.45 * targetSR when enabled
 * Signal flow: Input → (AA filter) → SRR → BRR → Output
 */
class Digital
{
public:
    Digital();
    ~Digital() = default;

    void prepare(double sampleRate, int blockSize, int numChannels);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, 
                 juce::AudioPlayHead* playHead, 
                 juce::AudioProcessorValueTreeState& apvts,
                 const MacroController& macro) noexcept;

private:
    // Enable 1st-order noise shaping for low bit depths
    static constexpr bool ENABLE_NOISE_SHAPING = true;
    static constexpr int NOISE_SHAPING_THRESHOLD = 8; // bits
    
    // Biquad filter coefficients
    struct BiquadCoeffs
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };
    
    // Per-channel state
    struct ChannelState
    {
        // SRR state
        double phase = 0.0;                // Phase accumulator for strobe
        float previousInput = 0.0f;        // Previous input for interpolation
        float heldSample = 0.0f;           // Current held sample value
        float lastPhaseIncrement = 0.0f;   // Track phase increment changes
        
        // BRR state  
        float noiseShapingError = 0.0f;    // Error feedback for noise shaping
        
        // Frequency-selective bit reduction state
        float highpassState = 0.0f;        // 1-pole highpass filter state
        float previousInputForHP = 0.0f;    // Previous input for highpass filter
        float lowpassState = 0.0f;          // 1st 1-pole lowpass filter state for warmth
        float lowpassState2 = 0.0f;         // 2nd 1-pole lowpass filter state for extra smoothing
        
        // Anti-alias filter state
        float x1 = 0.0f, x2 = 0.0f;        // Input delay line
        float y1 = 0.0f, y2 = 0.0f;        // Output delay line
        
        // Smoothed parameters
        juce::SmoothedValue<float> smoothedBits;
        juce::SmoothedValue<float> smoothedSampleRate;
        juce::SmoothedValue<float> smoothedCutoff;
    };
    
    std::vector<ChannelState> channels;
    
    // Global state
    double hostSampleRate = 44100.0;
    int numChannels = 2;
    juce::Random random;
    
    // Current filter coefficients
    BiquadCoeffs currentCoeffs;
    float lastCutoffFreq = 0.0f;
    
    // Processing methods
    float processBiquadFilter(ChannelState& channel, float input) noexcept;
    float processSampleRateReduction(ChannelState& channel, float input, 
                                    float targetSR, float jitterAmount) noexcept;
    float processBitDepthReduction(ChannelState& channel, float input, 
                                  float bits) noexcept;
    float processHardQuantization(ChannelState& channel, float input, 
                                 float bits) noexcept;
    
    // Helper methods
    void updateBiquadCoeffs(float cutoffFreq) noexcept;
    float generateTPDFDither(float stepSize) noexcept;
    float generateJitterOffset(float amount) noexcept;
    
    // Parameter mapping
    static float mapBitsToSmoothed(int bits) noexcept;
    static float mapSampleRateLog(float normalized) noexcept;
    
    // Legacy compatibility methods
    float generateDither() noexcept;
    float quantizeToBits(float input, int bits, float dither) noexcept;
    float processAntiAlias(ChannelState& channel, float input, float cutoffFreq) noexcept;
    float generateJitter(ChannelState& channel, float jitterAmount) noexcept;
    
    // Simplified interface methods
    float processSimpleAntiAlias(ChannelState& channel, float input, float cutoffFreq) noexcept;
    float generateSimpleJitter(ChannelState& channel, float amount) noexcept;
    float generateSimpleDither() noexcept;
    float quantizeWithDither(float input, int bits, float dither) noexcept;
    float quantizeHard(float input, int bits) noexcept;
    
    // New parallel processing methods (for compatibility)
    float processSampleRateReduction(ChannelState& channel, float input, 
                                    float targetSR, bool useAntiAlias, 
                                    float jitterAmount) noexcept;
    float processBitDepthReduction(ChannelState& channel, float input, 
                                  int targetBits) noexcept;
    float processAntiAliasingFilter(ChannelState& channel, float input, 
                                   float cutoffFreq) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Digital)
};

}