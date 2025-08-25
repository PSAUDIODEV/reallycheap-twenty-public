#pragma once

#include <JuceHeader.h>

namespace ReallyCheap
{
    
struct ParameterIDs
{
    static constexpr const char* inGain = "inGain";
    static constexpr const char* outGain = "outGain";
    static constexpr const char* mix = "mix";
    static constexpr const char* macroReallyCheap = "macroReallyCheap";
    static constexpr const char* bypass = "bypass";
    
    static constexpr const char* noiseOn = "noiseOn";
    static constexpr const char* noiseType = "noiseType";
    static constexpr const char* noiseLevel = "noiseLevel";
    static constexpr const char* noiseAge = "noiseAge";
    static constexpr const char* noiseFlutterGate = "noiseFlutterGate";
    static constexpr const char* noiseWidth = "noiseWidth";
    static constexpr const char* noisePlacement = "noisePlacement";
    
    static constexpr const char* wobbleOn = "wobbleOn";
    static constexpr const char* wobbleDepth = "wobbleDepth";
    static constexpr const char* wobbleRateHz = "wobbleRateHz";
    static constexpr const char* wobbleSync = "wobbleSync";
    static constexpr const char* wobbleFlutter = "wobbleFlutter";
    static constexpr const char* wobbleDrift = "wobbleDrift";
    static constexpr const char* wobbleJitter = "wobbleJitter";
    static constexpr const char* wobbleStereoLink = "wobbleStereoLink";
    static constexpr const char* wobbleMono = "wobbleMono";
    
    static constexpr const char* distortOn = "distortOn";
    static constexpr const char* distortType = "distortType";
    static constexpr const char* distortDrive = "distortDrive";
    static constexpr const char* distortTone = "distortTone";
    static constexpr const char* distortPrePost = "distortPrePost";
    
    static constexpr const char* digitalOn = "digitalOn";
    static constexpr const char* digitalBits = "digitalBits";
    static constexpr const char* digitalSR = "digitalSR";
    static constexpr const char* digitalJitter = "digitalJitter";
    static constexpr const char* digitalAA = "digitalAA";
    
    static constexpr const char* spaceOn = "spaceOn";
    static constexpr const char* spaceMix = "spaceMix";
    static constexpr const char* spaceTime = "spaceTime";
    static constexpr const char* spaceTone = "spaceTone";
    static constexpr const char* spacePreDelayMs = "spacePreDelayMs";
    static constexpr const char* spaceCheapo = "spaceCheapo";
    
    static constexpr const char* magOn = "magOn";
    static constexpr const char* magComp = "magComp";
    static constexpr const char* magSat = "magSat";
    static constexpr const char* magHeadBumpHz = "magHeadBumpHz";
    static constexpr const char* magCrosstalk = "magCrosstalk";
    static constexpr const char* magWear = "magWear";
};

struct ParameterDefaults
{
    static constexpr float inGain = 0.0f;
    static constexpr float outGain = 0.0f;
    static constexpr float mix = 0.5f;
    static constexpr float macroReallyCheap = 0.3f;
    static constexpr bool bypass = false;
    
    static constexpr bool noiseOn = false;
    static constexpr int noiseType = 0; // vinyl
    static constexpr float noiseLevel = -18.0f;  // Increased from -24dB for better audibility
    static constexpr float noiseAge = 0.4f;
    static constexpr float noiseFlutterGate = 0.15f;
    static constexpr float noiseWidth = 0.8f;
    static constexpr int noisePlacement = 0; // pre
    
    static constexpr bool wobbleOn = true;
    static constexpr float wobbleDepth = 0.2f;
    static constexpr float wobbleRateHz = 1.2f;
    static constexpr bool wobbleSync = true;
    static constexpr float wobbleFlutter = 0.15f;
    static constexpr float wobbleDrift = 0.25f;
    static constexpr float wobbleJitter = 0.1f;
    static constexpr float wobbleStereoLink = 0.7f;
    static constexpr bool wobbleMono = false;
    
    static constexpr bool distortOn = true;
    static constexpr int distortType = 0; // tape
    static constexpr float distortDrive = 4.0f;
    static constexpr float distortTone = 0.0f;
    static constexpr int distortPrePost = 1; // post
    
    static constexpr bool digitalOn = false;
    static constexpr int digitalBits = 12;
    static constexpr float digitalSR = 24000.0f;
    static constexpr float digitalJitter = 0.1f;
    static constexpr bool digitalAA = true;
    
    static constexpr bool spaceOn = true;
    static constexpr float spaceMix = 0.18f;
    static constexpr float spaceTime = 0.25f;
    static constexpr float spaceTone = 0.0f;
    static constexpr float spacePreDelayMs = 5.0f;
    static constexpr float spaceCheapo = 0.4f;
    
    static constexpr bool magOn = true;
    static constexpr float magComp = 0.3f;
    static constexpr float magSat = 0.25f;
    static constexpr float magHeadBumpHz = 70.0f;
    static constexpr float magCrosstalk = 0.2f;
    static constexpr float magWear = 0.2f;
};

class ParameterHelper
{
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    static juce::StringArray getNoiseTypeChoices() {
        return { "vinyl", "tape", "hum", "fan", "jazzClub" }; // Removed storePA
    }
    
    static juce::StringArray getDistortTypeChoices() {
        return { "tape", "diode", "fold" };
    }
    
    
    static juce::StringArray getPlacementChoices() {
        return { "pre", "post" };
    }
    
    static float decibelToLinear(float dB) {
        return juce::Decibels::decibelsToGain(dB);
    }
    
    static float linearToDecibel(float linear) {
        return juce::Decibels::gainToDecibels(linear);
    }
    
private:
    ParameterHelper() = delete;
};

}