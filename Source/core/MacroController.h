#pragma once

#include <JuceHeader.h>

namespace ReallyCheap
{

/**
 * Centralized Macro Controller that reads macroReallyCheap and computes
 * per-module modulation factors with musical curves and guardrails.
 * 
 * Thread-safe for audio thread usage. Does NOT modify APVTS parameters,
 * only provides scaling factors for modules to apply internally.
 */
class MacroController
{
public:
    MacroController() = default;
    ~MacroController() = default;
    
    /**
     * Prepare the macro controller for audio processing.
     * Called from prepareToPlay on audio thread.
     */
    void prepare(double sampleRate, int samplesPerBlock) noexcept;
    
    /**
     * Reset internal state.
     * Called from releaseResources.
     */
    void reset() noexcept;
    
    /**
     * Update macro state by reading APVTS macro value.
     * Called from processBlock on audio thread before module processing.
     * Updates internal smoothed scalars.
     */
    void tick(const juce::AudioProcessorValueTreeState& apvts) noexcept;
    
    // Getters for module scaling factors (audio thread safe)
    // These return pre-smoothed, bounded scalars
    
    // Wobble modulation (primary response)
    float wobbleDepthGain() const noexcept { return wobbleDepthGain_; }
    float wobbleFlutterGain() const noexcept { return wobbleFlutterGain_; }
    
    // Magnetic modulation (primary response)  
    float magneticCompGain() const noexcept { return magneticCompGain_; }
    float magneticSatGain() const noexcept { return magneticSatGain_; }
    
    // Distort modulation (secondary response)
    float distortDriveAddDb() const noexcept { return distortDriveAddDb_; }
    
    // Digital modulation (secondary response)
    float digitalBitsFloor() const noexcept { return digitalBitsFloor_; }
    float digitalSRFloorHz() const noexcept { return digitalSRFloorHz_; }
    
    // Space modulation (secondary response)
    float spaceMixCap() const noexcept { return spaceMixCap_; }
    
    // Noise modulation (secondary response)
    float noiseLevelAddDb() const noexcept { return noiseLevelAddDb_; }
    float noiseAgeGain() const noexcept { return noiseAgeGain_; }
    
    // Debug/utility
    float getCurrentMacroValue() const noexcept { return smoothedMacro_; }
    
private:
    double sampleRate_ = 44100.0;
    
    // Smoothed macro value (heavily smoothed ~80ms to avoid zippering)
    float smoothedMacro_ = 0.0f;
    float macroSmoothingCoeff_ = 0.0f;
    
    // Computed scaling factors (updated in tick())
    float wobbleDepthGain_ = 1.0f;
    float wobbleFlutterGain_ = 1.0f;
    float magneticCompGain_ = 1.0f;
    float magneticSatGain_ = 1.0f;
    float distortDriveAddDb_ = 0.0f;
    float digitalBitsFloor_ = 16.0f;
    float digitalSRFloorHz_ = 44100.0f;
    float spaceMixCap_ = 1.0f;
    float noiseLevelAddDb_ = 0.0f;
    float noiseAgeGain_ = 1.0f;
    
    // Musical easing functions
    static float ease(float x) noexcept;
    static float ease2(float x) noexcept;
    static float saturate(float x) noexcept;
    static float lerp(float a, float b, float t) noexcept;
    
    // Compute all scaling factors from smoothed macro value
    void updateScalingFactors() noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroController)
};

}