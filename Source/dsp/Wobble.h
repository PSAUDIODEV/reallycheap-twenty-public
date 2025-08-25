#pragma once

#include <JuceHeader.h>

namespace ReallyCheap
{

// Forward declaration
class MacroController;

class Wobble
{
public:
    Wobble();
    ~Wobble() = default;
    
    void prepare(double sampleRate, int blockSize, int numChannels);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, 
                 juce::AudioPlayHead* playHead, 
                 juce::AudioProcessorValueTreeState& apvts,
                 const MacroController& macro) noexcept;

private:
    // Core state
    double sampleRate = 44100.0;
    int numChannels = 2;
    
    // Simplified channel state for cleaner implementation
    struct ChannelState
    {
        // LFO phase
        double lfoPhase = 0.0;
        
        // Modulation smoothing
        float prevModValue = 0.0f;
        float jitterSmooth = 0.0f;
        
        // Circular delay buffer
        std::vector<float> delayLine;
        int delayWritePos = 0;
        int delaySize = 0;
        
        // Crossfade state for smooth parameter changes
        float crossfadeAmount = 0.0f;
        float oldReadPos = 0.0f;
        float newReadPos = 0.0f;
        
        // Anti-aliasing filter state (2nd order Butterworth)
        float lpf_x1 = 0.0f;
        float lpf_x2 = 0.0f;
        float lpf_y1 = 0.0f;
        float lpf_y2 = 0.0f;
    };
    
    std::vector<ChannelState> channels;
    
    // Random number generation (RT-safe)
    juce::Random random;
    
    // Legacy methods (kept for compatibility but not used)
    float calculateLfoValue(ChannelState& channel, bool useSync, float rateHz) noexcept;
    void updateDrift(ChannelState& channel, float driftAmount) noexcept;
    float generateJitter(ChannelState& channel, float jitterAmount) noexcept;
    void updateTempo(juce::AudioPlayHead* playHead) noexcept;
    double getSyncedFrequency(float rateHz, double bpm) const noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Wobble)
};

}