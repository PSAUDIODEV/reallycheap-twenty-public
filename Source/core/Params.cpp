#include "Params.h"

namespace ReallyCheap
{

juce::AudioProcessorValueTreeState::ParameterLayout ParameterHelper::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Global Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::inGain, "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f), ParameterDefaults::inGain));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::outGain, "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f), ParameterDefaults::outGain));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::mix, "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::mix));
    
    // Macro parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::macroReallyCheap, "Really Cheap Macro",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::macroReallyCheap));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::bypass, "Bypass", ParameterDefaults::bypass));
    
    
    // Noise Parameters
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::noiseOn, "Atmosphere On", ParameterDefaults::noiseOn));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterIDs::noiseType, "Atmosphere Type", getNoiseTypeChoices(), ParameterDefaults::noiseType));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::noiseLevel, "Atmosphere Level",
        juce::NormalisableRange<float>(-60.0f, -6.0f), ParameterDefaults::noiseLevel));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::noiseAge, "Atmosphere Age",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::noiseAge));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::noiseFlutterGate, "Atmosphere Flutter",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::noiseFlutterGate));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::noiseWidth, "Atmosphere Width",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::noiseWidth));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterIDs::noisePlacement, "Atmosphere Placement", getPlacementChoices(), ParameterDefaults::noisePlacement));
    
    // Wobble Parameters
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::wobbleOn, "Bend On", ParameterDefaults::wobbleOn));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::wobbleDepth, "Bend Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::wobbleDepth));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::wobbleRateHz, "Bend Rate",
        juce::NormalisableRange<float>(0.1f, 12.0f), ParameterDefaults::wobbleRateHz));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::wobbleSync, "Bend Sync", ParameterDefaults::wobbleSync));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::wobbleFlutter, "Bend Flutter",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::wobbleFlutter));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::wobbleDrift, "Bend Drift",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::wobbleDrift));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::wobbleJitter, "Bend Jitter",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::wobbleJitter));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::wobbleStereoLink, "Bend Stereo Link",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::wobbleStereoLink));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::wobbleMono, "Bend Mono", ParameterDefaults::wobbleMono));
    
    // Distort Parameters
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::distortOn, "Crunch On", ParameterDefaults::distortOn));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterIDs::distortType, "Crunch Type", getDistortTypeChoices(), ParameterDefaults::distortType));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::distortDrive, "Crunch Drive",
        juce::NormalisableRange<float>(0.0f, 12.0f), ParameterDefaults::distortDrive));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::distortTone, "Crunch Tone/Bias",
        juce::NormalisableRange<float>(-1.0f, 1.0f), ParameterDefaults::distortTone));
    
    
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterIDs::distortPrePost, "Crunch Pre/Post", getPlacementChoices(), ParameterDefaults::distortPrePost));
    
    // Digital Parameters
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::digitalOn, "Bitcrush On", ParameterDefaults::digitalOn));
    
    layout.add(std::make_unique<juce::AudioParameterInt>(
        ParameterIDs::digitalBits, "Bitcrush Bits", 4, 16, ParameterDefaults::digitalBits));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::digitalSR, "Bitcrush Sample Rate",
        juce::NormalisableRange<float>(6000.0f, 44100.0f), ParameterDefaults::digitalSR));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::digitalJitter, "Bitcrush Jitter",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::digitalJitter));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::digitalAA, "Bitcrush Anti-Aliasing", ParameterDefaults::digitalAA));
    
    // Space Parameters
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::spaceOn, "Verb On", ParameterDefaults::spaceOn));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::spaceMix, "Verb Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::spaceMix));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::spaceTime, "Verb Time",
        juce::NormalisableRange<float>(0.1f, 0.6f), ParameterDefaults::spaceTime));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::spaceTone, "Verb Tone",
        juce::NormalisableRange<float>(-1.0f, 1.0f), ParameterDefaults::spaceTone));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::spacePreDelayMs, "Verb Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 30.0f), ParameterDefaults::spacePreDelayMs));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::spaceCheapo, "Verb Cheapo",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::spaceCheapo));
    
    // Magnetic Parameters
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::magOn, "Tape On", ParameterDefaults::magOn));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::magComp, "Tape Compression",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::magComp));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::magSat, "Tape Saturation",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::magSat));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::magHeadBumpHz, "Tape Head Bump",
        juce::NormalisableRange<float>(40.0f, 120.0f), ParameterDefaults::magHeadBumpHz));
    
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::magCrosstalk, "Tape Crosstalk",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::magCrosstalk));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::magWear, "Tape Aging",
        juce::NormalisableRange<float>(0.0f, 1.0f), ParameterDefaults::magWear));
    
    return layout;
}

}