#include "SpaceIRManager.h"

namespace ReallyCheap
{

//==============================================================================
// SpaceIRManager
//==============================================================================

SpaceIRManager& SpaceIRManager::getInstance()
{
    static SpaceIRManager instance;
    return instance;
}

void SpaceIRManager::loadIRsFromFolder(const juce::File& folder)
{
    // This must be called from the message thread only
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    
    // Create fallback IR if we don't have one
    if (!fallbackIR)
    {
        fallbackIR = generateFallbackIR(44100.0, 0.25f); // 250ms fallback
        DBG("SpaceIRManager: Generated fallback IR - " << fallbackIR->buffer.getNumSamples() << " samples, " << fallbackIR->buffer.getNumChannels() << " channels");
    }
    
    // Create new IR collection
    pendingIRs = std::make_unique<IRCollection>();
    
    if (!folder.exists())
    {
        DBG("Space IR folder does not exist: " << folder.getFullPathName());
        // Swap in empty collection so we use fallback
        irPointer.store(pendingIRs.get());
        activeIRs = std::move(pendingIRs);
        return;
    }
    
    // Scan for WAV files
    auto wavFiles = folder.findChildFiles(juce::File::findFiles, false, "*.wav");
    
    size_t totalSize = 0;
    const size_t maxTotalSize = 4 * 1024 * 1024; // 4MB limit
    
    for (const auto& file : wavFiles)
    {
        auto filename = file.getFileName();
        
        // Check if filename matches expected pattern (spring_*, room_*)
        if (!filename.toLowerCase().startsWith("spring_") && 
            !filename.toLowerCase().startsWith("room_"))
        {
            continue;
        }
        
        // Check size budget
        auto fileSize = file.getSize();
        if (totalSize + fileSize > maxTotalSize)
        {
            DBG("Skipping " << filename << " - would exceed 4MB budget");
            continue;
        }
        
        auto irBuffer = loadIRFile(file);
        if (irBuffer)
        {
            // Check duration (max 300ms)
            float duration = irBuffer->buffer.getNumSamples() / static_cast<float>(irBuffer->sampleRate);
            if (duration > 0.3f)
            {
                DBG("IR " << filename << " is too long (" << duration << "s), truncating to 300ms");
                int maxSamples = static_cast<int>(0.3f * irBuffer->sampleRate);
                irBuffer->buffer.setSize(irBuffer->buffer.getNumChannels(), maxSamples, true, true, true);
                irBuffer->lengthSeconds = 0.3f;
            }
            
            totalSize += fileSize;
            
            // Categorize and store
            if (irBuffer->category == "spring")
                pendingIRs->springIRs.push_back(irBuffer.get());
            else if (irBuffer->category == "room")
                pendingIRs->roomIRs.push_back(irBuffer.get());
            
            pendingIRs->allIRs.push_back(std::move(irBuffer));
            
            DBG("Loaded IR: " << filename << " (" << fileSize << " bytes, " 
                << duration << "s, " << irBuffer->category << ")");
        }
    }
    
    // Atomically swap to new IRs (RT-safe)
    irPointer.store(pendingIRs.get());
    activeIRs = std::move(pendingIRs);
    
    DBG("Total IRs loaded: " << activeIRs->allIRs.size() << " (" << totalSize << " bytes)");
}

const SpaceIRManager::IRBuffer* SpaceIRManager::getIR(int index) const noexcept
{
    auto* irs = irPointer.load();
    if (!irs || index < 0 || index >= static_cast<int>(irs->allIRs.size()))
        return fallbackIR.get();
    
    return irs->allIRs[index].get();
}

const SpaceIRManager::IRBuffer* SpaceIRManager::getSpringIR(int variant) const noexcept
{
    auto* irs = irPointer.load();
    if (!irs || irs->springIRs.empty())
        return fallbackIR.get();
    
    int index = variant % static_cast<int>(irs->springIRs.size());
    return irs->springIRs[index];
}

const SpaceIRManager::IRBuffer* SpaceIRManager::getRoomIR(int variant) const noexcept
{
    auto* irs = irPointer.load();
    if (!irs || irs->roomIRs.empty())
        return fallbackIR.get();
    
    int index = variant % static_cast<int>(irs->roomIRs.size());
    return irs->roomIRs[index];
}

const SpaceIRManager::IRBuffer* SpaceIRManager::getBestIRForTime(float timeSeconds) const noexcept
{
    auto* irs = irPointer.load();
    if (!irs || irs->allIRs.empty())
        return fallbackIR.get();
    
    // Find IR with length closest to requested time
    float bestDiff = 10.0f;
    const IRBuffer* bestIR = nullptr;
    
    for (const auto& ir : irs->allIRs)
    {
        float diff = std::abs(ir->lengthSeconds - timeSeconds);
        if (diff < bestDiff)
        {
            bestDiff = diff;
            bestIR = ir.get();
        }
    }
    
    return bestIR ? bestIR : fallbackIR.get();
}

size_t SpaceIRManager::getTotalIRCount() const noexcept
{
    auto* irs = irPointer.load();
    return irs ? irs->allIRs.size() : 0;
}

size_t SpaceIRManager::getSpringIRCount() const noexcept
{
    auto* irs = irPointer.load();
    return irs ? irs->springIRs.size() : 0;
}

size_t SpaceIRManager::getRoomIRCount() const noexcept
{
    auto* irs = irPointer.load();
    return irs ? irs->roomIRs.size() : 0;
}

bool SpaceIRManager::needsFallbackIR() const noexcept
{
    auto* irs = irPointer.load();
    return !irs || irs->allIRs.empty();
}

std::unique_ptr<SpaceIRManager::IRBuffer> SpaceIRManager::generateFallbackIR(double sampleRate, float lengthSeconds)
{
    auto buffer = std::make_unique<IRBuffer>();
    buffer->sampleRate = sampleRate;
    buffer->lengthSeconds = lengthSeconds;
    buffer->filename = "fallback_ir";
    buffer->category = "room";
    
    int numSamples = static_cast<int>(lengthSeconds * sampleRate);
    buffer->buffer.setSize(2, numSamples); // Stereo
    
    // Generate decaying noise burst
    FallbackIRGenerator::generateDecayingNoise(buffer->buffer, sampleRate, lengthSeconds);
    
    // Apply room-like coloration
    FallbackIRGenerator::applyRoomColoration(buffer->buffer, sampleRate);
    
    return buffer;
}

std::unique_ptr<SpaceIRManager::IRBuffer> SpaceIRManager::loadIRFile(const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (!reader)
        return nullptr;
    
    auto buffer = std::make_unique<IRBuffer>();
    buffer->filename = file.getFileName();
    buffer->category = getCategoryFromFilename(buffer->filename);
    buffer->sampleRate = reader->sampleRate;
    buffer->lengthSeconds = static_cast<float>(reader->lengthInSamples) / static_cast<float>(reader->sampleRate);
    
    buffer->buffer.setSize(static_cast<int>(reader->numChannels), 
                           static_cast<int>(reader->lengthInSamples));
    
    reader->read(&buffer->buffer, 0, 
                 static_cast<int>(reader->lengthInSamples), 
                 0, true, true);
    
    // Convert to stereo if mono
    if (buffer->buffer.getNumChannels() == 1)
    {
        buffer->buffer.setSize(2, buffer->buffer.getNumSamples(), true, true, true);
        buffer->buffer.copyFrom(1, 0, buffer->buffer, 0, 0, buffer->buffer.getNumSamples());
    }
    
    return buffer;
}

juce::String SpaceIRManager::getCategoryFromFilename(const juce::String& filename)
{
    auto lower = filename.toLowerCase();
    
    if (lower.startsWith("spring"))
        return "spring";
    else if (lower.startsWith("room"))
        return "room";
    
    return "unknown";
}

//==============================================================================
// FallbackIRGenerator
//==============================================================================

void FallbackIRGenerator::generateDecayingNoise(juce::AudioBuffer<float>& buffer, 
                                                double sampleRate, 
                                                float lengthSeconds,
                                                float decayTime) noexcept
{
    juce::ignoreUnused(lengthSeconds);
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    uint32_t seed = 12345;
    
    // Generate decaying noise
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        uint32_t chSeed = seed + ch * 6789; // Different seed per channel
        
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(sampleRate);
            
            // Exponential decay envelope
            float envelope = std::exp(-t / decayTime);
            
            // Generate white noise
            float noise = (fastRand(chSeed) / 2147483648.0f) - 1.0f; // -1 to 1
            
            data[i] = noise * envelope * 1.0f; // Boost amplitude for more audible reverb
        }
    }
}

void FallbackIRGenerator::applySpringColoration(juce::AudioBuffer<float>& buffer, 
                                               double sampleRate) noexcept
{
    // Spring reverb characteristics: resonant peaks, metallic coloration
    SimpleFilter lpf1, lpf2, hpf;
    lpf1.setLowpass(sampleRate, 3000.0f);
    lpf2.setLowpass(sampleRate, 8000.0f);
    hpf.setHighpass(sampleRate, 150.0f);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float sample = data[i];
            
            // High-pass to remove low end
            sample = hpf.process(sample);
            
            // Low-pass cascade for warmth
            sample = lpf2.process(lpf1.process(sample));
            
            // Add slight metallic resonance
            sample *= (1.0f + 0.1f * std::sin(i * 0.01f));
            
            data[i] = sample;
        }
    }
}

void FallbackIRGenerator::applyRoomColoration(juce::AudioBuffer<float>& buffer, 
                                             double sampleRate) noexcept
{
    // Room reverb characteristics: natural decay, wider frequency response
    SimpleFilter lpf, hpf;
    lpf.setLowpass(sampleRate, 12000.0f);
    hpf.setHighpass(sampleRate, 40.0f);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float sample = data[i];
            
            // Gentle high and low pass
            sample = lpf.process(hpf.process(sample));
            
            data[i] = sample;
        }
    }
}

uint32_t FallbackIRGenerator::fastRand(uint32_t& seed) noexcept
{
    seed = seed * 1103515245 + 12345;
    return seed;
}

void FallbackIRGenerator::SimpleFilter::setLowpass(double sampleRate, float freq)
{
    coeff = std::exp(-2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate));
}

void FallbackIRGenerator::SimpleFilter::setHighpass(double sampleRate, float freq)
{
    coeff = std::exp(-2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate));
}

float FallbackIRGenerator::SimpleFilter::process(float input) noexcept
{
    state = input + coeff * (state - input);
    return state;
}

}