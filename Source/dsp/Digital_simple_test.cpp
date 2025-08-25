#include "Digital.h"
#include "../core/Params.h"
#include "../core/MacroController.h"

namespace ReallyCheap
{

Digital::Digital()
{
    random.setSeedRandomly();
}

void Digital::prepare(double sampleRate_, int blockSize, int numChannels_)
{
    juce::ignoreUnused(sampleRate_, blockSize, numChannels_);
    // Minimal preparation for test
}

void Digital::reset()
{
    // Minimal reset for test
}

void Digital::process(juce::AudioBuffer<float>& buffer, 
                     juce::AudioPlayHead* playHead, 
                     juce::AudioProcessorValueTreeState& apvts,
                     const MacroController& macro) noexcept
{
    juce::ignoreUnused(buffer, playHead, apvts, macro);
    
    // COMPLETE BYPASS FOR TESTING
    // If you still hear bitcrushing effects with this version,
    // the issue is definitely NOT in Digital.cpp
    return;
}

// Minimal implementations of other required methods
float Digital::generateDither() noexcept { return 0.0f; }
float Digital::quantizeToBits(float input, int, float) noexcept { return input; }
float Digital::processAntiAlias(ChannelState&, float input, float) noexcept { return input; }
float Digital::generateJitter(ChannelState&, float) noexcept { return 0.0f; }
float Digital::processSimpleAntiAlias(ChannelState& channel, float input, float cutoffFreq) noexcept { return input; }
float Digital::generateSimpleJitter(ChannelState&, float) noexcept { return 0.0f; }
float Digital::generateSimpleDither() noexcept { return 0.0f; }
float Digital::quantizeWithDither(float input, int, float) noexcept { return input; }
float Digital::quantizeHard(float input, int) noexcept { return input; }
float Digital::processSampleRateReduction(ChannelState&, float input, float, bool, float) noexcept { return input; }
float Digital::processBitDepthReduction(ChannelState&, float input, int) noexcept { return input; }
float Digital::processAntiAliasingFilter(ChannelState&, float input, float) noexcept { return input; }

}