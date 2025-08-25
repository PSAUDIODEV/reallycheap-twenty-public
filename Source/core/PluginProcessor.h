#pragma once

#include <JuceHeader.h>
#include "Params.h"
#include "Presets.h"
#include "MacroController.h"
#include "../dsp/Distort.h"
#include "../dsp/Wobble.h"
#include "../dsp/Digital.h"
#include "../dsp/Magnetic.h"
#include "../dsp/Noise.h"
#include "../dsp/Space.h"

class ReallyCheapTwentyAudioProcessor : public juce::AudioProcessor
{
public:
    ReallyCheapTwentyAudioProcessor();
    ~ReallyCheapTwentyAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; }
    
    // Preset management methods
    bool loadFactoryPreset(const juce::String& name);
    bool loadUserPreset(const juce::String& path);
    bool saveUserPreset(const juce::String& path);
    juce::StringArray getPresetList();
    ReallyCheap::PresetManager& getPresetManager() { return presetManager; }

private:
    juce::AudioProcessorValueTreeState valueTreeState;
    ReallyCheap::PresetManager presetManager;
    ReallyCheap::MacroController macroController;
    
    std::atomic<float>* inGainParam = nullptr;
    std::atomic<float>* outGainParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;
    
    juce::SmoothedValue<float> inGainSmoothed;
    juce::SmoothedValue<float> outGainSmoothed;
    juce::SmoothedValue<float> mixSmoothed;
    
    ReallyCheap::Distort distort;
    ReallyCheap::Wobble wobble;
    ReallyCheap::Digital digital;
    ReallyCheap::Magnetic magnetic;
    ReallyCheap::Noise noise;
    ReallyCheap::Space space;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReallyCheapTwentyAudioProcessor)
};