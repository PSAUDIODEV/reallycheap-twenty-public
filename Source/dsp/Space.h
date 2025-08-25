#pragma once

#include <JuceHeader.h>

namespace ReallyCheap
{

// Forward declaration
class MacroController;

class Space
{
public:
    Space();
    
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    
    void process(juce::AudioBuffer<float>& buffer,
                juce::AudioPlayHead* playHead,
                juce::AudioProcessorValueTreeState& apvts,
                const MacroController& macro) noexcept;
    
    // Get latency for processor-wide compensation
    int getLatencySamples() const noexcept;
    
    // Message-thread call to load/reload IRs (legacy - not used in algorithmic version)
    static void requestIRPreload(const juce::File& folder);
    
private:
    double sampleRate = 44100.0;
    int blockSize = 512;
    int numChannels = 2;
    
    // Pre-delay line
    struct PreDelayLine
    {
        std::vector<float> buffer;
        int writePos = 0;
        int maxDelaySamples = 0;
        
        void prepare(double sampleRate, int maxDelayMs);
        void reset();
        float processSample(float input, float delaySamples) noexcept;
    };
    
    std::vector<PreDelayLine> preDelayLines;
    
    // Tone control (tilt EQ)
    struct TiltEQ
    {
        juce::dsp::IIR::Filter<float> lowShelf;
        juce::dsp::IIR::Filter<float> highShelf;
        
        void prepare(double sampleRate);
        void updateCoeffs(float tiltAmount, double sampleRate);
        float processSample(float input) noexcept;
        void reset();
    };
    
    std::vector<TiltEQ> tiltEQs;
    
    // Algorithmic reverb structures
    std::vector<std::vector<std::vector<float>>> reverbDelays; // [channel][tap][delay_buffer]
    
    struct ReverbState
    {
        int writePos = 0;
        float feedback = 0.6f;
        float diffusion = 0.5f;
        float lowpass1 = 0.0f;
        float lowpass2 = 0.0f;
        float allpass1 = 0.0f;
        float allpass2 = 0.0f;
    };
    
    std::vector<ReverbState> reverbState;
    
    // Parameter smoothing
    juce::SmoothedValue<float> mixSmoothed;
    juce::SmoothedValue<float> preDelaySmoothed;
    juce::SmoothedValue<float> toneSmoothed;
    juce::SmoothedValue<float> reverbTimeSmoothed;
    juce::SmoothedValue<float> roomSizeSmoothed;
    
    // Temporary buffers
    juce::AudioBuffer<float> wetBuffer;
    juce::AudioBuffer<float> dryBuffer;
    
    // Processing methods
    void processAlgorithmicReverb(juce::AudioBuffer<float>& buffer) noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Space)
};

}