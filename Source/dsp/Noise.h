#pragma once

#include <JuceHeader.h>
#include "noise/NoiseAssetManager.h"
#include <array>

namespace ReallyCheap
{

// Forward declaration
class MacroController;

class Noise
{
public:
    Noise();
    
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    
    // Processes in-place by mixing noise into buffer
    void process(juce::AudioBuffer<float>& buffer,
                juce::AudioPlayHead* playHead,
                juce::AudioProcessorValueTreeState& apvts,
                const MacroController& macro) noexcept;
    
    // Called from message thread to load/replace assets safely
    static void requestAssetPreload(const juce::File& folder);
    
private:
    double sampleRate = 44100.0;
    int numChannels = 2;
    
    // Grain engine state per channel
    struct GrainState
    {
        // Current playback position in source buffer
        double readPosition = 0.0;
        
        // Grain window position (0-1)
        float grainPhase = 0.0f;
        
        // Next grain start position
        double nextGrainPosition = 0.0;
        
        // Crossfade state
        float crossfadePhase = 0.0f;
        bool inCrossfade = false;
        double crossfadeStartPos = 0.0;
        double crossfadeEndPos = 0.0;
        
        // Random offset for this grain
        float currentOffset = 0.0f;
        
        // Per-channel random seed for deterministic variation
        uint32_t randSeed = 0;
    };
    
    std::vector<GrainState> grainStates;
    
    // Procedural fallback generator
    std::unique_ptr<ProceduralNoiseGenerator> proceduralGen;
    
    // Current noise type for asset selection
    NoiseAssetManager::NoiseType currentNoiseType = NoiseAssetManager::NoiseType::Vinyl;
    
    // Parameter smoothing
    juce::SmoothedValue<float> levelSmoothed;
    juce::SmoothedValue<float> ageSmoothed;
    juce::SmoothedValue<float> widthSmoothed;
    juce::SmoothedValue<float> flutterGateSmoothed;
    
    // Age filter state (per channel)
    struct AgeFilterState
    {
        juce::dsp::IIR::Filter<float> highpass;
        juce::dsp::IIR::Filter<float> lowpass;
        juce::dsp::IIR::Filter<float> midDip; // Gentle mid scoop for aged sound
    };
    std::vector<AgeFilterState> ageFilters;
    
    // Flutter gate state
    struct FlutterGateState
    {
        float envelopeState = 0.0f;
        float flutterPhase = 0.0f;
        float gateCoeff = 1.0f;
    };
    FlutterGateState flutterGate;
    
    // Grain engine parameters
    static constexpr float grainSizeMs = 80.0f; // 80ms grains
    static constexpr float crossfadeSizeMs = 15.0f; // 15ms crossfades
    static constexpr float maxOffsetMs = 100.0f; // ±100ms random offset
    
    // Helper functions
    float getHannWindow(float phase) const noexcept;
    float getNextRandomOffset(GrainState& state) const noexcept;
    void updateAgeFilters(float ageAmount);
    void updateFlutterGate(const juce::AudioBuffer<float>& inputBuffer, int numSamples);
    float applyFlutterGate(float input, float gateAmount) noexcept;
    void applyWidthProcessing(float& left, float& right, float width) noexcept;
    
    // Get interpolated sample from buffer with wrapping
    float getInterpolatedSample(const juce::AudioBuffer<float>& buffer, 
                                int channel, double position) const noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Noise)
};

}