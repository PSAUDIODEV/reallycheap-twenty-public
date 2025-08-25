#pragma once

#include <JuceHeader.h>

namespace ReallyCheap
{

// Forward declaration
class MacroController;

class Magnetic
{
public:
    Magnetic();
    ~Magnetic() = default;
    
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
    
    // Per-channel processing state
    struct ChannelState
    {
        // Compression envelope follower (2-pole)
        float compEnvState1 = 0.0f;
        float compEnvState2 = 0.0f;
        float lastGainReduction = 0.0f;
        
        // Pre-emphasis/de-emphasis filters for saturation
        juce::dsp::IIR::Filter<float> preEmphasisFilter;
        juce::dsp::IIR::Filter<float> deEmphasisFilter;
        
        // Head bump low-shelf filter
        juce::dsp::IIR::Filter<float> headBumpFilter;
        
        // Wear high-frequency rolloff filter
        juce::dsp::IIR::Filter<float> wearFilter;
        
        // Crosstalk delay for stereo bleed
        std::array<float, 8> crosstalkDelay = {}; // Small delay buffer
        int crosstalkWritePos = 0;
    };
    
    std::vector<ChannelState> channels;
    
    // Parameter smoothing
    juce::SmoothedValue<float> smoothedCompAmount;
    juce::SmoothedValue<float> smoothedSatAmount;
    juce::SmoothedValue<float> smoothedCrosstalk;
    juce::SmoothedValue<float> smoothedHeadBump;
    juce::SmoothedValue<float> smoothedWear;
    
    // Hiss generation
    juce::Random random;
    
    // Internal methods
    float processCompression(ChannelState& channel, float input, float compAmount) noexcept;
    float processSaturation(ChannelState& channel, float input, float satAmount) noexcept;
    void applyCrosstalk(juce::AudioBuffer<float>& buffer, float crosstalkAmount) noexcept;
    void updateHeadBumpFilter(ChannelState& channel, float headBumpAmount);
    void updateWearFilter(ChannelState& channel, float wearAmount);
    float softClip(float input) noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Magnetic)
};

}