#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../dsp/Noise.h"

ReallyCheapTwentyAudioProcessor::ReallyCheapTwentyAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                       .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
    , valueTreeState(*this, nullptr, "Parameters", ReallyCheap::ParameterHelper::createParameterLayout())
    , presetManager(valueTreeState)
{
    inGainParam = valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::inGain);
    outGainParam = valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::outGain);
    mixParam = valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::mix);
    bypassParam = valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::bypass);
    
    // Initialize with embedded assets (full functionality restored)
    try
    {
        // Load default preset (Subtle Glue)
        presetManager.loadFactoryPreset("Subtle Glue");
        presetManager.setCurrentPresetIndex(0);
        
        // Load noise assets from embedded binary data (no external files needed!)
        DBG("Loading embedded noise assets...");
        ReallyCheap::NoiseAssetManager::getInstance().loadAssetsFromBinaryData();
        DBG("Plugin processor initialized successfully");
    }
    catch (...)
    {
        DBG("Exception during plugin processor initialization - continuing");
    }
}

ReallyCheapTwentyAudioProcessor::~ReallyCheapTwentyAudioProcessor()
{
}

const juce::String ReallyCheapTwentyAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReallyCheapTwentyAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ReallyCheapTwentyAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ReallyCheapTwentyAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ReallyCheapTwentyAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReallyCheapTwentyAudioProcessor::getNumPrograms()
{
    return 1;
}

int ReallyCheapTwentyAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReallyCheapTwentyAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String ReallyCheapTwentyAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void ReallyCheapTwentyAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void ReallyCheapTwentyAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // FL Studio compatibility - validate parameters
    if (sampleRate <= 0 || samplesPerBlock <= 0)
    {
        DBG("Invalid audio parameters - FL Studio compatibility fallback");
        return; // Don't crash FL Studio with invalid params
    }
    
    inGainSmoothed.reset(sampleRate, 0.02);
    outGainSmoothed.reset(sampleRate, 0.02);
    mixSmoothed.reset(sampleRate, 0.03);
    
    inGainSmoothed.setCurrentAndTargetValue(ReallyCheap::ParameterHelper::decibelToLinear(*inGainParam));
    outGainSmoothed.setCurrentAndTargetValue(ReallyCheap::ParameterHelper::decibelToLinear(*outGainParam));
    mixSmoothed.setCurrentAndTargetValue(*mixParam);
    
    macroController.prepare(sampleRate, samplesPerBlock);
    
    distort.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    wobble.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    digital.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    magnetic.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    noise.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    space.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
}

void ReallyCheapTwentyAudioProcessor::releaseResources()
{
    macroController.reset();
    
    distort.reset();
    wobble.reset();
    digital.reset();
    magnetic.reset();
    noise.reset();
    space.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReallyCheapTwentyAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void ReallyCheapTwentyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    
    // Basic debug output to verify debug messages are working
    static int processBlockCount = 0;
    processBlockCount++;
    if (processBlockCount < 5)
    {
        DBG("*** PROCESS BLOCK #" << processBlockCount << " - Plugin is running! ***");
    }
    
    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // Bypass removed - DAWs handle this natively

    // Update macro controller before processing any modules
    macroController.tick(valueTreeState);

    inGainSmoothed.setTargetValue(ReallyCheap::ParameterHelper::decibelToLinear(*inGainParam));
    outGainSmoothed.setTargetValue(ReallyCheap::ParameterHelper::decibelToLinear(*outGainParam));
    mixSmoothed.setTargetValue(*mixParam);

    // Apply input gain
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float inGain = inGainSmoothed.getNextValue();
            channelData[sample] *= inGain;
        }
    }

    // Store dry signal for mix
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Check noise placement (pre or post effects)
    const int noisePlacement = static_cast<int>(*valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::noisePlacement));
    
    
    // Apply pre-effect noise if configured
    if (noisePlacement == 0) // 0 = pre
    {
        noise.process(buffer, getPlayHead(), valueTreeState, macroController);
    }
    
    // Check distort placement setting
    const int distortPlacement = static_cast<int>(*valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::distortPrePost));
    
    // Apply distortion PRE if configured
    if (distortPlacement == 0) // 0 = pre (before wobble)
    {
        distort.process(buffer, getPlayHead(), valueTreeState, macroController);
    }
    
    // Process wobble (wow/flutter) first for vintage character
    wobble.process(buffer, getPlayHead(), valueTreeState, macroController);

    // Apply distortion POST if configured  
    if (distortPlacement == 1) // 1 = post (after wobble, default)
    {
        distort.process(buffer, getPlayHead(), valueTreeState, macroController);
    }
    
    // Process digital degradation
    static int mainProcessorCallCount = 0;
    mainProcessorCallCount++;
    if (mainProcessorCallCount < 10)
    {
        DBG("=== MAIN PROCESSOR DEBUG #" << mainProcessorCallCount << " ===");
        DBG("About to call digital.process() - buffer has " << buffer.getNumSamples() << 
            " samples, " << buffer.getNumChannels() << " channels");
        
        // Check digital module state
        const bool digitalOn = *valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::digitalOn) > 0.5f;
        const int digitalBits = static_cast<int>(*valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::digitalBits));
        const float digitalSR = *valueTreeState.getRawParameterValue(ReallyCheap::ParameterIDs::digitalSR);
        
        DBG("Digital params before call - On: " << digitalOn << ", Bits: " << digitalBits << ", SR: " << digitalSR);
    }
    
    digital.process(buffer, getPlayHead(), valueTreeState, macroController);
    
    if (mainProcessorCallCount < 10)
    {
        DBG("Digital.process() completed");
    }
    
    // Process magnetic tape characteristics
    magnetic.process(buffer, getPlayHead(), valueTreeState, macroController);
    
    // Apply post-effect noise if configured
    if (noisePlacement == 1) // 1 = post
    {
        noise.process(buffer, getPlayHead(), valueTreeState, macroController);
    }
    
    // Apply space (reverb) at the end of the chain
    space.process(buffer, getPlayHead(), valueTreeState, macroController);

    // Apply mix and output gain
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* wetData = buffer.getWritePointer(channel);
        auto* dryData = dryBuffer.getReadPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float mix = mixSmoothed.getNextValue();
            const float outGain = outGainSmoothed.getNextValue();
            
            float outputSample = (wetData[sample] * mix) + (dryData[sample] * (1.0f - mix));
            outputSample *= outGain;
            
            // EMERGENCY SAFETY LIMITER - prevent feedback damage
            outputSample = juce::jlimit(-2.0f, 2.0f, outputSample);
            if (!std::isfinite(outputSample))
                outputSample = 0.0f;
            
            wetData[sample] = outputSample;
        }
    }
}

bool ReallyCheapTwentyAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ReallyCheapTwentyAudioProcessor::createEditor()
{
    return new ReallyCheapTwentyAudioProcessorEditor(*this);
}

void ReallyCheapTwentyAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ReallyCheapTwentyAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(valueTreeState.state.getType()))
            valueTreeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Preset management methods
bool ReallyCheapTwentyAudioProcessor::loadFactoryPreset(const juce::String& name)
{
    return presetManager.loadFactoryPreset(name);
}

bool ReallyCheapTwentyAudioProcessor::loadUserPreset(const juce::String& path)
{
    juce::File presetFile(path);
    return presetManager.loadPreset(presetFile);
}

bool ReallyCheapTwentyAudioProcessor::saveUserPreset(const juce::String& path)
{
    juce::File presetFile(path);
    auto presetName = presetFile.getFileNameWithoutExtension();
    return presetManager.savePreset(presetName, presetFile);
}

juce::StringArray ReallyCheapTwentyAudioProcessor::getPresetList()
{
    return presetManager.getAllPresetNames();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReallyCheapTwentyAudioProcessor();
}