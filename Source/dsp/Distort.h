#pragma once

#include <JuceHeader.h>
#include "../core/Params.h"

namespace ReallyCheap
{

// Forward declaration
class MacroController;

class Distort
{
public:
    Distort();
    ~Distort() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, juce::AudioPlayHead* playHead, 
                juce::AudioProcessorValueTreeState& apvts, const MacroController& macro) noexcept;
    
    int getLatencySamples() const noexcept { return latencySamples; }
    void setBypassed(bool shouldBeBypassed) noexcept { bypassed = shouldBeBypassed; }

private:
    enum class DistortType
    {
        Tape = 0,
        Diode = 1,
        Fold = 2,
        Tape2x = 3,
        Diode2x = 4,
        Fold2x = 5,
        Tape4x = 6,
        Diode4x = 7,
        Fold4x = 8
    };

    enum class OversamplingFactor
    {
        x1 = 0,
        x2 = 1,
        x4 = 2
    };

    void updateParameters(juce::AudioProcessorValueTreeState& apvts, const MacroController& macro) noexcept;
    void processInternal(juce::AudioBuffer<float>& buffer) noexcept;
    
    float processTapeMode(float input) noexcept;
    float processDiodeMode(float input) noexcept;
    float processFoldMode(float input) noexcept;
    
    void applyPreEmphasis(juce::AudioBuffer<float>& buffer) noexcept;
    void applyDeEmphasis(juce::AudioBuffer<float>& buffer) noexcept;
    void applyToneShaping(juce::AudioBuffer<float>& buffer, bool isPreShaper) noexcept;
    void applyBias(juce::AudioBuffer<float>& buffer) noexcept;
    void removeDC(juce::AudioBuffer<float>& buffer) noexcept;

    double sampleRate = 44100.0;
    int maxSamplesPerBlock = 512;
    int numChannels = 2;
    int latencySamples = 0;
    bool bypassed = false;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    juce::AudioBuffer<float> oversampledBuffer;
    juce::AudioBuffer<float> dryDelayBuffer;
    
    juce::dsp::IIR::Filter<float> preEmphasisFilters[2];
    juce::dsp::IIR::Filter<float> deEmphasisFilters[2];  
    juce::dsp::IIR::Filter<float> toneFilters[2];
    juce::dsp::IIR::Filter<float> dcBlockFilters[2];
    
    int dryDelayWritePos = 0;
    
    DistortType currentType = DistortType::Tape;
    OversamplingFactor currentOS = OversamplingFactor::x2;
    float currentDrive = 1.0f;
    float currentTone = 0.0f;
    float currentBias = 0.0f;
    
    static constexpr float kMaxDriveGain = 15.85f;
    static constexpr float kDCBlockFreq = 5.0f;
    static constexpr float kPreEmphasisFreq = 3000.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Distort)
};

}