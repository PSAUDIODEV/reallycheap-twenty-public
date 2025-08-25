#include "Noise.h"
#include "../core/Params.h"
#include "../core/MacroController.h"

namespace ReallyCheap
{

Noise::Noise()
{
    proceduralGen = std::make_unique<ProceduralNoiseGenerator>();
}

void Noise::prepare(double sampleRate_, int samplesPerBlock, int numChannels_)
{
    juce::ignoreUnused(samplesPerBlock);
    sampleRate = sampleRate_;
    numChannels = numChannels_;
    
    // Setup grain states for each channel
    grainStates.clear();
    grainStates.resize(numChannels);
    
    // Initialize with deterministic seeds per channel
    for (int ch = 0; ch < numChannels; ++ch)
    {
        grainStates[ch].randSeed = 12345 + ch * 6789;
        grainStates[ch].readPosition = 0.0;
        grainStates[ch].grainPhase = 0.0f;
        grainStates[ch].nextGrainPosition = 0.0;
        grainStates[ch].currentOffset = 0.0f;
    }
    
    // Setup procedural generator
    proceduralGen->prepare(sampleRate, samplesPerBlock);
    
    // Setup parameter smoothing
    const double smoothTime = 0.02; // 20ms
    levelSmoothed.reset(sampleRate, smoothTime);
    ageSmoothed.reset(sampleRate, smoothTime * 2); // Slower for filters
    widthSmoothed.reset(sampleRate, smoothTime);
    flutterGateSmoothed.reset(sampleRate, smoothTime * 3); // Slower for gate
    
    // Setup age filters
    ageFilters.clear();
    ageFilters.resize(numChannels);
    
    for (auto& filter : ageFilters)
    {
        // Initialize with neutral settings
        auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
        filter.highpass.coefficients = hpCoeffs;
        
        auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 20000.0f);
        filter.lowpass.coefficients = lpCoeffs;
        
        // Mid dip for aged sound (bell at 2kHz, Q=0.5, -3dB)
        auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, 2000.0f, 0.5f, juce::Decibels::decibelsToGain(-3.0f));
        filter.midDip.coefficients = midCoeffs;
    }
    
    reset();
}

void Noise::reset()
{
    for (auto& state : grainStates)
    {
        state.readPosition = 0.0;
        // Reset other state variables (not needed for simple looping but keep for compatibility)
        state.grainPhase = 0.0f;
        state.inCrossfade = false;
        state.crossfadePhase = 0.0f;
    }
    
    for (auto& filter : ageFilters)
    {
        filter.highpass.reset();
        filter.lowpass.reset();
        filter.midDip.reset();
    }
    
    flutterGate.envelopeState = 0.0f;
    flutterGate.flutterPhase = 0.0f;
    flutterGate.gateCoeff = 1.0f;
    
    proceduralGen->reset();
    
    // NOTE: Don't initialize smoothers here - they will be initialized on first process() call
    // with actual parameter values from APVTS
}

void Noise::process(juce::AudioBuffer<float>& buffer,
                   juce::AudioPlayHead* playHead,
                   juce::AudioProcessorValueTreeState& apvts,
                   const MacroController& macro) noexcept
{
    juce::ignoreUnused(playHead);
    
    const int numSamples = buffer.getNumSamples();
    const int bufferChannels = buffer.getNumChannels();
    
    // Get parameters
    const bool noiseOn = *apvts.getRawParameterValue(ParameterIDs::noiseOn) > 0.5f;
    
    // Debug output for first few calls
    static int debugCallCount = 0;
    if (debugCallCount < 5)
    {
        DBG("Noise::process - Call " << debugCallCount << ", noiseOn: " << (noiseOn ? "YES" : "NO") << ", numSamples: " << numSamples);
        debugCallCount++;
    }
    
    if (!noiseOn)
        return;
    
    const int noiseTypeInt = static_cast<int>(*apvts.getRawParameterValue(ParameterIDs::noiseType));
    const float baseLevelDb = *apvts.getRawParameterValue(ParameterIDs::noiseLevel);
    const float age = *apvts.getRawParameterValue(ParameterIDs::noiseAge);
    const float width = *apvts.getRawParameterValue(ParameterIDs::noiseWidth);
    const float flutterGateAmount = *apvts.getRawParameterValue(ParameterIDs::noiseFlutterGate);
    
    // Debug parameter reading
    static int paramDebugCount = 0;
    if (paramDebugCount < 10)
    {
        DBG("RAW noiseTypeInt from parameter: " << noiseTypeInt);
        paramDebugCount++;
    }
    
    // Apply macro modulation with guardrails
    const float levelDb = juce::jlimit(-60.0f, 12.0f, baseLevelDb + macro.noiseLevelAddDb());
    
    // Update noise type
    currentNoiseType = static_cast<NoiseAssetManager::NoiseType>(
        juce::jlimit(0, static_cast<int>(NoiseAssetManager::NoiseType::NumTypes) - 1, noiseTypeInt));
        
    if (paramDebugCount < 10)
    {
        DBG("After limits - currentNoiseType: " << static_cast<int>(currentNoiseType));
    }
    
    // Initialize smoothers on first call (they weren't initialized in reset())
    static bool smoothersInitialized = false;
    if (!smoothersInitialized)
    {
        levelSmoothed.setCurrentAndTargetValue(ParameterHelper::decibelToLinear(levelDb));
        ageSmoothed.setCurrentAndTargetValue(age);
        widthSmoothed.setCurrentAndTargetValue(width);
        flutterGateSmoothed.setCurrentAndTargetValue(flutterGateAmount);
        smoothersInitialized = true;
        DBG("Noise smoothers initialized - levelDb: " << levelDb << ", linear: " << ParameterHelper::decibelToLinear(levelDb));
    }
    
    // Update smoothed parameters
    levelSmoothed.setTargetValue(ParameterHelper::decibelToLinear(levelDb));
    ageSmoothed.setTargetValue(age);
    widthSmoothed.setTargetValue(width);
    flutterGateSmoothed.setTargetValue(flutterGateAmount);
    
    // Update age filters once per block
    updateAgeFilters(ageSmoothed.getCurrentValue());
    
    // Update flutter gate envelope
    updateFlutterGate(buffer, numSamples);
    
    // Check if we need procedural fallback
    auto& assetManager = NoiseAssetManager::getInstance();
    bool useProcedural = assetManager.needsProceduralFallback(currentNoiseType);
    const NoiseAssetManager::AssetBuffer* assetBuffer = nullptr;
    
    // Debug: Show why we're using procedural or not
    static int pathDebugCount = 0;
    if (pathDebugCount < 3)
    {
        DBG("Noise type " << static_cast<int>(currentNoiseType) << " - isProceduralType: " << 
            (NoiseAssetManager::isProceduralType(currentNoiseType) ? "YES" : "NO"));
        pathDebugCount++;
    }
    
    if (!useProcedural)
    {
        assetBuffer = assetManager.getAssetForType(currentNoiseType);
        useProcedural = (assetBuffer == nullptr);
    }
    
    // Debug output for processing path
    static int processDebugCount = 0;
    if (processDebugCount < 5)
    {
        DBG("=== NOISE DEBUG ===");
        DBG("Type: " << static_cast<int>(currentNoiseType) << " (" << 
            (currentNoiseType == NoiseAssetManager::NoiseType::Vinyl ? "Vinyl" :
             currentNoiseType == NoiseAssetManager::NoiseType::Tape ? "Tape" :
             currentNoiseType == NoiseAssetManager::NoiseType::Hum ? "Hum" :
             currentNoiseType == NoiseAssetManager::NoiseType::Fan ? "Fan" :
             currentNoiseType == NoiseAssetManager::NoiseType::JazzClub ? "JazzClub" : "Unknown") << ")");
        DBG("needsProceduralFallback: " << (assetManager.needsProceduralFallback(currentNoiseType) ? "YES" : "NO"));
        DBG("UseProcedural: " << (useProcedural ? "YES" : "NO"));
        DBG("AssetBuffer: " << (assetBuffer ? "VALID" : "NULL"));
        if (assetBuffer)
        {
            DBG("  Asset filename: " << assetBuffer->filename);
            DBG("  Asset length: " << assetBuffer->buffer.getNumSamples() << " samples");
        }
        processDebugCount++;
    }
    
    // Temporary buffers for noise generation
    juce::AudioBuffer<float> noiseBuffer(2, numSamples);
    noiseBuffer.clear();
    
    if (useProcedural)
    {
        // Use procedural generator
        proceduralGen->generateNoise(currentNoiseType,
                                    noiseBuffer.getWritePointer(0),
                                    noiseBuffer.getWritePointer(1),
                                    numSamples);
    }
    else
    {
        // Use simple seamless looping with loaded assets and optimal loop points
        const auto& sourceBuffer = assetBuffer->buffer;
        const int sourceLength = sourceBuffer.getNumSamples();
        const int loopStart = assetBuffer->loopStartSample;
        const int loopEnd = assetBuffer->loopEndSample;
        const int loopLength = loopEnd - loopStart;
        const double assetSampleRate = assetBuffer->sampleRate;
        
        // Calculate sample rate ratio for correct playback speed
        const double sampleRateRatio = assetSampleRate / sampleRate;
        
        // Debug sample rate info
        static int srDebugCount = 0;
        if (srDebugCount < 3)
        {
            DBG("Asset sample rate: " << assetSampleRate << "Hz, Plugin sample rate: " << sampleRate << "Hz, Ratio: " << sampleRateRatio);
            srDebugCount++;
        }
        
        if (sourceLength > 0 && loopLength > 0)
        {
            for (int ch = 0; ch < std::min(2, bufferChannels); ++ch)
            {
                auto& grain = grainStates[ch]; // Reuse state structure for simple playback position
                auto* noiseOut = noiseBuffer.getWritePointer(ch);
                
                for (int sample = 0; sample < numSamples; ++sample)
                {
                    // Simple linear playback with seamless looping
                    double position = loopStart + grain.readPosition;
                    
                    // Get interpolated sample
                    float outputSample = getInterpolatedSample(sourceBuffer, 
                                                             ch % sourceBuffer.getNumChannels(),
                                                             position);
                    
                    // Apply per-type level adjustments
                    if (currentNoiseType == NoiseAssetManager::NoiseType::JazzClub)
                    {
                        outputSample *= 0.4f; // Jazz club quieter (60% reduction)
                    }
                    
                    noiseOut[sample] = outputSample;
                    
                    // Advance read position with sample rate compensation and wrap at loop end
                    grain.readPosition += sampleRateRatio; // Correct playback speed
                    if (grain.readPosition >= loopLength)
                        grain.readPosition = 0.0; // Loop back to start
                }
            }
        }
    }
    
    // Process noise through Age filters and effects
    for (int ch = 0; ch < std::min(2, bufferChannels); ++ch)
    {
        auto* noiseData = noiseBuffer.getWritePointer(ch);
        auto& ageFilter = ageFilters[ch];
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Apply age filtering
            float processed = noiseData[sample];
            processed = ageFilter.highpass.processSample(processed);
            processed = ageFilter.lowpass.processSample(processed);
            processed = ageFilter.midDip.processSample(processed);
            
            // Apply flutter gate
            processed = applyFlutterGate(processed, flutterGateSmoothed.getNextValue());
            
            // Apply level
            processed *= levelSmoothed.getNextValue();
            
            noiseData[sample] = processed;
        }
    }
    
    // Apply width processing (M/S)
    if (bufferChannels >= 2)
    {
        auto* leftData = noiseBuffer.getWritePointer(0);
        auto* rightData = noiseBuffer.getWritePointer(1);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float left = leftData[sample];
            float right = rightData[sample];
            
            applyWidthProcessing(left, right, widthSmoothed.getNextValue());
            
            leftData[sample] = left;
            rightData[sample] = right;
        }
    }
    
    // Debug: Check noise signal levels before mixing
    static int mixDebugCount = 0;
    if (mixDebugCount < 3)
    {
        float maxLevel = 0.0f;
        for (int ch = 0; ch < noiseBuffer.getNumChannels(); ++ch)
        {
            auto magnitude = noiseBuffer.getMagnitude(ch, 0, numSamples);
            maxLevel = std::max(maxLevel, magnitude);
        }
        DBG("Noise signal magnitude before mixing: " << maxLevel << " (should be > 0 if working)");
        mixDebugCount++;
    }
    
    // Mix noise into output buffer
    for (int ch = 0; ch < std::min(bufferChannels, noiseBuffer.getNumChannels()); ++ch)
    {
        buffer.addFrom(ch, 0, noiseBuffer, ch, 0, numSamples);
    }
    
    // Flush denormals
    for (int ch = 0; ch < bufferChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            if (!std::isfinite(data[sample]) || std::abs(data[sample]) < 1e-30f)
                data[sample] = 0.0f;
        }
    }
}

void Noise::requestAssetPreload(const juce::File& folder)
{
    // This must be called from the message thread
    NoiseAssetManager::getInstance().loadAssetsFromFolder(folder);
}

float Noise::getHannWindow(float phase) const noexcept
{
    phase = juce::jlimit(0.0f, 1.0f, phase);
    return 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * phase));
}

float Noise::getNextRandomOffset(GrainState& state) const noexcept
{
    // Simple LCG for deterministic random
    state.randSeed = state.randSeed * 1103515245 + 12345;
    float rand = (state.randSeed & 0x7fffffff) / 2147483648.0f;
    return (rand - 0.5f) * 2.0f; // -1 to 1
}

void Noise::updateAgeFilters(float ageAmount)
{
    // Map age 0..1 to filter parameters
    // HPF: 20Hz -> 120Hz
    // LPF: 20kHz -> 6kHz  
    // Mid dip: 0dB -> -6dB
    
    float hpFreq = 20.0f + ageAmount * 100.0f;
    float lpFreq = 20000.0f - ageAmount * 14000.0f;
    float midGain = juce::Decibels::decibelsToGain(-ageAmount * 6.0f);
    
    auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, hpFreq);
    auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lpFreq);
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, 2000.0f, 0.5f, midGain);
    
    for (auto& filter : ageFilters)
    {
        filter.highpass.coefficients = hpCoeffs;
        filter.lowpass.coefficients = lpCoeffs;
        filter.midDip.coefficients = midCoeffs;
    }
}

void Noise::updateFlutterGate(const juce::AudioBuffer<float>& inputBuffer, int numSamples)
{
    // Calculate input RMS for gate trigger
    float rms = 0.0f;
    for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch)
    {
        const float* data = inputBuffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            rms += data[i] * data[i];
        }
    }
    rms = std::sqrt(rms / (numSamples * inputBuffer.getNumChannels()));
    
    // Envelope follower (fast attack, slow release)
    float attackCoeff = std::exp(-1.0f / (0.01f * static_cast<float>(sampleRate))); // 10ms
    float releaseCoeff = std::exp(-1.0f / (0.2f * static_cast<float>(sampleRate))); // 200ms
    
    float coeff = (rms > flutterGate.envelopeState) ? attackCoeff : releaseCoeff;
    flutterGate.envelopeState = rms + coeff * (flutterGate.envelopeState - rms);
    
    // Update flutter LFO
    float flutterRate = 6.0f; // 6Hz flutter
    flutterGate.flutterPhase += flutterRate * numSamples / static_cast<float>(sampleRate);
    if (flutterGate.flutterPhase >= 1.0f)
        flutterGate.flutterPhase -= 1.0f;
}

float Noise::applyFlutterGate(float input, float gateAmount) noexcept
{
    if (gateAmount <= 0.0f)
        return input;
    
    // Gate reduction based on input level (duck when loud)
    float gateReduction = juce::jlimit(0.0f, 1.0f, flutterGate.envelopeState * 4.0f);
    gateReduction = 1.0f - (gateReduction * 0.5f * gateAmount); // Max 50% reduction
    
    // Add flutter modulation
    float flutter = std::sin(2.0f * juce::MathConstants<float>::pi * flutterGate.flutterPhase);
    flutter = 1.0f + (flutter * 0.1f * gateAmount); // ±10% flutter
    
    return input * gateReduction * flutter;
}

void Noise::applyWidthProcessing(float& left, float& right, float width) noexcept
{
    // M/S processing
    float mid = (left + right) * 0.5f;
    float side = (left - right) * 0.5f;
    
    // Width control: 0 = mono (side=0), 1 = original stereo
    side *= width;
    
    // Convert back to L/R
    left = mid + side;
    right = mid - side;
}

float Noise::getInterpolatedSample(const juce::AudioBuffer<float>& buffer, 
                                  int channel, double position) const noexcept
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return 0.0f;
    
    // Wrap position
    while (position >= numSamples)
        position -= numSamples;
    while (position < 0)
        position += numSamples;
    
    int index1 = static_cast<int>(position);
    int index2 = (index1 + 1) % numSamples;
    float fraction = static_cast<float>(position - index1);
    
    const float* data = buffer.getReadPointer(channel);
    return data[index1] + fraction * (data[index2] - data[index1]);
}

}