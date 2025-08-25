#include "Distort.h"
#include "../core/MacroController.h"
#include <cmath>

namespace ReallyCheap
{

Distort::Distort() = default;

void Distort::prepare(double sampleRate_, int samplesPerBlock, int numChannels_)
{
    sampleRate = sampleRate_;
    maxSamplesPerBlock = samplesPerBlock;
    numChannels = numChannels_;

    // 4x oversampling for all modes to prevent aliasing
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        numChannels, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, 
        true, false);
    
    oversampler->initProcessing(samplesPerBlock);
    latencySamples = static_cast<int>(oversampler->getLatencyInSamples());
    
    oversampledBuffer.setSize(numChannels, samplesPerBlock * 4);
    dryDelayBuffer.setSize(numChannels, samplesPerBlock + latencySamples + 64);
    dryDelayBuffer.clear();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate * 4; // Oversampled rate for filters
    spec.maximumBlockSize = samplesPerBlock * 4;
    spec.numChannels = 1;

    for (int ch = 0; ch < 2; ++ch)
    {
        auto& preEmph = preEmphasisFilters[ch];
        auto& deEmph = deEmphasisFilters[ch];
        auto& tone = toneFilters[ch];
        auto& dcBlock = dcBlockFilters[ch];

        preEmph.prepare(spec);
        deEmph.prepare(spec);
        tone.prepare(spec);
        dcBlock.prepare(spec);

        // Gentler pre-emphasis for smoother saturation
        preEmph.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate * 4, 2000.0f, 0.5f, 1.2f); // Reduced boost
        deEmph.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate * 4, 2000.0f, 0.5f, 1.0f / 1.2f);
        
        // Start with flat tone response
        tone.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate * 4, 1000.0f, 0.707f, 1.0f);
        
        // DC blocker
        dcBlock.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate * 4, 20.0f);

        preEmph.reset();
        deEmph.reset();
        tone.reset();
        dcBlock.reset();
    }

    dryDelayWritePos = 0;
}

void Distort::reset()
{
    if (oversampler)
        oversampler->reset();
    
    oversampledBuffer.clear();
    dryDelayBuffer.clear();
    
    for (int ch = 0; ch < 2; ++ch)
    {
        preEmphasisFilters[ch].reset();
        deEmphasisFilters[ch].reset();
        toneFilters[ch].reset();
        dcBlockFilters[ch].reset();
    }
    
    dryDelayWritePos = 0;
}

void Distort::process(juce::AudioBuffer<float>& buffer, juce::AudioPlayHead* playHead, 
                     juce::AudioProcessorValueTreeState& apvts, const MacroController& macro) noexcept
{
    juce::ignoreUnused(playHead);
    juce::ScopedNoDenormals noDenormals;
    
    updateParameters(apvts, macro);
    
    if (bypassed)
        return;
    
    // Process with 4x oversampling to reduce aliasing
    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampledBlock = oversampler->processSamplesUp(block);
    
    // Get oversampled buffer
    auto* oversampledData = oversampledBlock.getChannelPointer(0);
    const int oversampledNumSamples = static_cast<int>(oversampledBlock.getNumSamples());
    
    // Create temporary buffer for processing
    juce::AudioBuffer<float> tempBuffer(buffer.getNumChannels(), oversampledNumSamples);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* src = oversampledBlock.getChannelPointer(ch);
        auto* dst = tempBuffer.getWritePointer(ch);
        std::copy(src, src + oversampledNumSamples, dst);
    }
    
    // Process at oversampled rate
    processInternal(tempBuffer);
    
    // Copy back to oversampled block
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* src = tempBuffer.getReadPointer(ch);
        auto* dst = oversampledBlock.getChannelPointer(ch);
        std::copy(src, src + oversampledNumSamples, dst);
    }
    
    // Downsample back to original rate
    oversampler->processSamplesDown(block);
}

void Distort::updateParameters(juce::AudioProcessorValueTreeState& apvts, const MacroController& macro) noexcept
{
    auto* distortOnParam = apvts.getRawParameterValue(ParameterIDs::distortOn);
    setBypassed(!(distortOnParam->load() >= 0.5f));
    
    if (bypassed)
        return;

    auto* typeParam = apvts.getRawParameterValue(ParameterIDs::distortType);
    auto* driveParam = apvts.getRawParameterValue(ParameterIDs::distortDrive);
    auto* toneParam = apvts.getRawParameterValue(ParameterIDs::distortTone);

    // Simple type selection (0-2 for three types)
    int typeValue = static_cast<int>(typeParam->load());
    currentType = static_cast<DistortType>(juce::jlimit(0, 2, typeValue));
    
    // Apply macro modulation with guardrails
    float baseDriveDb = driveParam->load();
    float modifiedDriveDb = juce::jlimit(0.0f, 40.0f, baseDriveDb + macro.distortDriveAddDb());
    currentDrive = juce::Decibels::decibelsToGain(modifiedDriveDb);
    
    // Tone control only (no bias for cleaner sound)
    currentTone = toneParam->load();
    currentBias = 0.0f; // Remove bias to prevent DC offset artifacts
}

void Distort::processInternal(juce::AudioBuffer<float>& buffer) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    const auto channels = juce::jmin(buffer.getNumChannels(), numChannels);

    // Apply tone shaping before distortion
    applyToneShaping(buffer, true);
    
    // Apply gentle pre-emphasis
    applyPreEmphasis(buffer);

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        
        for (int i = 0; i < numSamples; ++i)
        {
            float input = data[i];
            float output = 0.0f;
            
            // Scale input by drive
            input *= currentDrive;
            
            // Apply selected distortion type
            switch (currentType)
            {
                case DistortType::Tape:
                    output = processTapeMode(input);
                    break;
                case DistortType::Diode:
                    output = processDiodeMode(input);
                    break;
                case DistortType::Fold:
                    output = processFoldMode(input);
                    break;
                default:
                    output = processTapeMode(input);
                    break;
            }
            
            // Apply output gain compensation based on drive amount
            float compensation = 1.0f / (1.0f + currentDrive * 0.3f);
            data[i] = output * compensation;
        }
    }

    // Remove DC offset
    removeDC(buffer);
    
    // Apply de-emphasis
    applyDeEmphasis(buffer);
}

float Distort::processTapeMode(float input) noexcept
{
    // Improved tape saturation algorithm with smoother curve
    // Based on modified hyperbolic tangent with asymmetry
    
    // Add subtle asymmetry for tape character
    float asymmetry = 0.05f;
    float biased = input + asymmetry * input * input;
    
    // Soft saturation using tanh approximation
    // This approximation is smoother and less harsh than standard tanh
    float x = biased * 0.7f; // Scale for gentler saturation
    float x2 = x * x;
    
    // Padé approximation of tanh for smoother saturation
    float num = x * (27.0f + x2);
    float den = 27.0f + 9.0f * x2;
    float output = num / den;
    
    // Add subtle even harmonics for warmth
    output += 0.02f * output * output * (input > 0 ? 1.0f : -1.0f);
    
    return output;
}

float Distort::processDiodeMode(float input) noexcept
{
    // Cleaner diode clipping with softer saturation
    // Models germanium characteristics with smoother transition
    
    const float threshold = 0.5f; // Lower threshold for gentler clipping
    const float softness = 0.8f; // Softer saturation curve
    
    // Simple asymmetric soft clipping using tanh-based approach
    float scaled = input * 1.5f; // Slight pre-gain
    
    // Asymmetric behavior: slightly different for positive/negative
    if (input >= 0.0f)
    {
        // Positive half - standard soft clipping
        if (input < threshold)
            return input;
        else
            return threshold + (1.0f - threshold) * std::tanh((input - threshold) * softness);
    }
    else
    {
        // Negative half - slightly harder clipping for asymmetry
        float negThreshold = threshold * 0.8f;
        if (input > -negThreshold)
            return input;
        else
            return -negThreshold - (1.0f - negThreshold) * std::tanh((-input - negThreshold) * softness * 1.2f);
    }
}

float Distort::processFoldMode(float input) noexcept
{
    // Much gentler wavefolder for musical distortion
    // Uses controlled triangle folding with soft transitions
    
    const float foldThreshold = 0.8f; // High threshold - less folding
    const float foldAmount = 0.3f; // Reduced intensity
    
    float absInput = std::abs(input);
    
    if (absInput <= foldThreshold)
    {
        // No folding in linear region
        return input;
    }
    else
    {
        // Gentle folding for signals above threshold
        float excess = absInput - foldThreshold;
        float foldedExcess = foldThreshold - excess * foldAmount;
        
        // Soft limit the folding to prevent extreme values
        foldedExcess = juce::jmax(foldedExcess, foldThreshold * 0.5f);
        
        // Apply sign and blend with original
        float folded = (input >= 0.0f) ? foldedExcess : -foldedExcess;
        
        // Blend original and folded for smoother transition
        float blend = juce::jlimit(0.0f, 0.6f, (absInput - foldThreshold) * 2.0f);
        return input * (1.0f - blend) + folded * blend;
    }
}

void Distort::applyPreEmphasis(juce::AudioBuffer<float>& buffer) noexcept
{
    const auto channels = juce::jmin(buffer.getNumChannels(), 2);
    
    for (int ch = 0; ch < channels; ++ch)
    {
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers() + ch, 1, buffer.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> context(block);
        preEmphasisFilters[ch].process(context);
    }
}

void Distort::applyDeEmphasis(juce::AudioBuffer<float>& buffer) noexcept
{
    const auto channels = juce::jmin(buffer.getNumChannels(), 2);
    
    for (int ch = 0; ch < channels; ++ch)
    {
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers() + ch, 1, buffer.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> context(block);
        deEmphasisFilters[ch].process(context);
    }
}

void Distort::applyToneShaping(juce::AudioBuffer<float>& buffer, bool isPreShaper) noexcept
{
    if (std::abs(currentTone) < 0.01f)
        return;
    
    const auto channels = juce::jmin(buffer.getNumChannels(), 2);
    const float oversampleRate = sampleRate * 4; // We're processing at 4x rate
    
    for (int ch = 0; ch < channels; ++ch)
    {
        // Gentler tone control: negative = darker, positive = brighter
        float freq = 1000.0f * std::pow(2.0f, currentTone * 1.5f); // ±1.5 octaves (reduced)
        float q = 0.5f; // Gentler Q
        float gain = 1.0f + std::abs(currentTone) * 1.5f; // Reduced max gain
        
        if (currentTone < 0)
        {
            // Cut highs for darker tone - use shelf throughout for consistency
            toneFilters[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                oversampleRate, freq, q, 1.0f / gain);
        }
        else
        {
            // Boost highs for brighter tone
            toneFilters[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                oversampleRate, freq, q, gain);
        }
        
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers() + ch, 1, buffer.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> context(block);
        toneFilters[ch].process(context);
    }
}

void Distort::applyBias(juce::AudioBuffer<float>& buffer) noexcept
{
    // Removed bias application to prevent DC offset artifacts
}

void Distort::removeDC(juce::AudioBuffer<float>& buffer) noexcept
{
    const auto channels = juce::jmin(buffer.getNumChannels(), 2);
    
    for (int ch = 0; ch < channels; ++ch)
    {
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers() + ch, 1, buffer.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> context(block);
        dcBlockFilters[ch].process(context);
    }
}

}