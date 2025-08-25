#include "NoiseAssetManager.h"
#include "BinaryData.h"

namespace ReallyCheap
{

//==============================================================================
// NoiseAssetManager
//==============================================================================

NoiseAssetManager& NoiseAssetManager::getInstance()
{
    static NoiseAssetManager instance;
    return instance;
}

void NoiseAssetManager::loadAssetsFromFolder(const juce::File& folder)
{
    // For backward compatibility, but we now prefer binary data
    loadAssetsFromBinaryData();
    
    // If binary data loading failed, fall back to file loading
    if (!activeAssets || activeAssets[static_cast<size_t>(NoiseType::Vinyl)].buffers.empty())
    {
        loadAssetsFromFolderInternal(folder);
    }
}

void NoiseAssetManager::loadAssetsFromBinaryData()
{
    // This must be called from the message thread only
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    
    DBG("Loading noise assets from binary data...");
    
    // Create new asset collection
    pendingAssets = std::make_unique<AssetCollection[]>(static_cast<size_t>(NoiseType::NumTypes));
    
    // Load each embedded asset
    struct BinaryAsset
    {
        const char* data;
        int size;
        const char* name;
        NoiseType type;
    };
    
    BinaryAsset assets[] = {
        { BinaryData::vinyl_mp3, BinaryData::vinyl_mp3Size, "vinyl.mp3", NoiseType::Vinyl },
        { BinaryData::tape_mp3, BinaryData::tape_mp3Size, "tape.mp3", NoiseType::Tape },
        { BinaryData::jazz_club_mp3, BinaryData::jazz_club_mp3Size, "jazz club.mp3", NoiseType::JazzClub }
    };
    
    size_t totalSize = 0;
    for (const auto& asset : assets)
    {
        DBG("Loading binary asset: " << asset.name << " (" << asset.size << " bytes)");
        
        auto assetBuffer = loadAudioFromMemory(asset.data, asset.size, asset.name);
        if (assetBuffer)
        {
            totalSize += asset.size;
            pendingAssets[static_cast<size_t>(asset.type)].buffers.push_back(std::move(assetBuffer));
            DBG("Successfully loaded: " << asset.name);
        }
        else
        {
            DBG("Failed to load: " << asset.name);
        }
    }
    
    // Atomically swap to new assets (RT-safe)
    assetPointer.store(pendingAssets.get());
    activeAssets = std::move(pendingAssets);
    
    // Debug: Show what was loaded for each type
    for (size_t i = 0; i < static_cast<size_t>(NoiseType::NumTypes); ++i)
    {
        size_t count = activeAssets[i].buffers.size();
        if (count > 0)
        {
            DBG("Type " << i << " has " << count << " binary assets loaded");
        }
        else
        {
            DBG("Type " << i << " has NO assets (will use procedural)");
        }
    }
    
    DBG("Total binary assets loaded: " << totalSize << " bytes");
}

void NoiseAssetManager::loadAssetsFromFolderInternal(const juce::File& folder)
{
    // This must be called from the message thread only
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    
    // Create new asset collection
    pendingAssets = std::make_unique<AssetCollection[]>(static_cast<size_t>(NoiseType::NumTypes));
    
    if (!folder.exists())
    {
        DBG("ERROR: Noise asset folder does not exist: " << folder.getFullPathName());
        // Swap in empty collection so we use procedural fallback
        assetPointer.store(pendingAssets.get());
        activeAssets = std::move(pendingAssets);
        return;
    }
    
    DBG("Asset folder exists! Scanning: " << folder.getFullPathName());
    
    // Scan for audio files matching our naming scheme (WAV and MP3)
    auto audioFiles = folder.findChildFiles(juce::File::findFiles, false, "*.wav;*.mp3");
    
    size_t totalSize = 0;
    const size_t maxTotalSize = 10 * 1024 * 1024; // 10MB limit
    
    for (const auto& file : audioFiles)
    {
        auto filename = file.getFileName();
        auto type = getTypeFromFilename(filename);
        
        DBG("Processing file: '" << filename << "' -> Type: " << static_cast<int>(type));
        
        if (type == NoiseType::NumTypes)
        {
            DBG("  Skipped - doesn't match naming scheme");
            continue; // Skip files that don't match our naming scheme
        }
        
        // Check size budget
        auto fileSize = file.getSize();
        if (totalSize + fileSize > maxTotalSize)
        {
            DBG("Skipping " << filename << " - would exceed 10MB budget");
            continue;
        }
        
        auto assetBuffer = loadAudioFile(file);
        if (assetBuffer)
        {
            totalSize += fileSize;
            pendingAssets[static_cast<size_t>(type)].buffers.push_back(std::move(assetBuffer));
            DBG("Loaded noise asset: " << filename << " (" << fileSize << " bytes)");
        }
    }
    
    // Atomically swap to new assets (RT-safe)
    assetPointer.store(pendingAssets.get());
    activeAssets = std::move(pendingAssets);
    
    // Debug: Show what was loaded for each type
    for (size_t i = 0; i < static_cast<size_t>(NoiseType::NumTypes); ++i)
    {
        size_t count = activeAssets[i].buffers.size();
        if (count > 0)
        {
            DBG("Type " << i << " has " << count << " assets loaded");
        }
        else
        {
            DBG("Type " << i << " has NO assets (will use procedural)");
        }
    }
    
    DBG("Total noise assets loaded: " << totalSize << " bytes");
}

const NoiseAssetManager::AssetBuffer* NoiseAssetManager::getAssetForType(NoiseType type) const noexcept
{
    static int getAssetDebugCount = 0;
    if (getAssetDebugCount < 10)
    {
        DBG("getAssetForType called with type: " << static_cast<int>(type));
        getAssetDebugCount++;
    }
    
    auto* assets = assetPointer.load();
    if (!assets)
    {
        if (getAssetDebugCount < 10) DBG("  No assets loaded");
        return nullptr;
    }
    
    auto typeIndex = static_cast<size_t>(type);
    if (typeIndex >= static_cast<size_t>(NoiseType::NumTypes))
    {
        if (getAssetDebugCount < 10) DBG("  Invalid type index: " << typeIndex);
        return nullptr;
    }
    
    auto& collection = assets[typeIndex];
    if (collection.buffers.empty())
    {
        if (getAssetDebugCount < 10) DBG("  No assets for type " << static_cast<int>(type));
        return nullptr;
    }
    
    // Return current buffer (could cycle through multiple if available)
    auto index = collection.currentIndex.load() % collection.buffers.size();
    if (getAssetDebugCount < 10)
    {
        DBG("  Returning asset for type " << static_cast<int>(type) << ": " << collection.buffers[index]->filename);
    }
    return collection.buffers[index].get();
}

size_t NoiseAssetManager::getAssetCountForType(NoiseType type) const noexcept
{
    auto* assets = assetPointer.load();
    if (!assets)
        return 0;
    
    auto typeIndex = static_cast<size_t>(type);
    if (typeIndex >= static_cast<size_t>(NoiseType::NumTypes))
        return 0;
    
    return assets[typeIndex].buffers.size();
}

bool NoiseAssetManager::needsProceduralFallback(NoiseType type) const noexcept
{
    // Always use procedural for Hum and Fan
    if (isProceduralType(type))
        return true;
        
    return getAssetCountForType(type) == 0;
}

bool NoiseAssetManager::isProceduralType(NoiseType type) noexcept
{
    return type == NoiseType::Hum || type == NoiseType::Fan;
}

std::unique_ptr<NoiseAssetManager::AssetBuffer> NoiseAssetManager::loadAudioFile(const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats(); // Supports WAV, AIFF, MP3, etc.
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (!reader)
    {
        DBG("Failed to create reader for: " << file.getFileName());
        return nullptr;
    }
    
    auto buffer = std::make_unique<AssetBuffer>();
    buffer->filename = file.getFileName();
    buffer->sampleRate = reader->sampleRate;
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
    
    // Find optimal zero-crossing loop points for seamless looping
    auto loopPoints = findZeroCrossingLoopPoints(buffer->buffer);
    buffer->loopStartSample = loopPoints.first;
    buffer->loopEndSample = loopPoints.second;
    
    DBG("Loaded " << file.getFileName() << " - Length: " << buffer->buffer.getNumSamples() 
        << " samples, Loop: " << buffer->loopStartSample << " to " << buffer->loopEndSample);
    
    return buffer;
}

std::unique_ptr<NoiseAssetManager::AssetBuffer> NoiseAssetManager::loadAudioFromMemory(const char* data, int size, const juce::String& filename)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats(); // Supports WAV, AIFF, MP3, etc.
    
    // Create a memory input stream
    auto memoryStream = std::make_unique<juce::MemoryInputStream>(data, static_cast<size_t>(size), false);
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(std::move(memoryStream)));
    if (!reader)
    {
        DBG("Failed to create reader for binary data: " << filename);
        return nullptr;
    }
    
    auto buffer = std::make_unique<AssetBuffer>();
    buffer->filename = filename;
    buffer->sampleRate = reader->sampleRate;
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
    
    // Find optimal zero-crossing loop points for seamless looping
    auto loopPoints = findZeroCrossingLoopPoints(buffer->buffer);
    buffer->loopStartSample = loopPoints.first;
    buffer->loopEndSample = loopPoints.second;
    
    DBG("Loaded binary data " << filename << " - Length: " << buffer->buffer.getNumSamples() 
        << " samples, Loop: " << buffer->loopStartSample << " to " << buffer->loopEndSample);
    
    return buffer;
}

std::pair<int, int> NoiseAssetManager::findZeroCrossingLoopPoints(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples < 1000)
    {
        // Too short for meaningful loop detection
        return {0, numSamples - 1};
    }
    
    // Find zero crossings in the first 10% and last 10% of the file
    const int searchRange = numSamples / 10;
    const float threshold = 0.001f; // Small threshold around zero
    
    auto findNearestZeroCrossing = [&](int startSearchFrom, int searchDirection) -> int 
    {
        // Mix both channels to mono for analysis
        for (int i = 0; i < searchRange; ++i)
        {
            int sampleIndex = startSearchFrom + (i * searchDirection);
            if (sampleIndex < 0 || sampleIndex >= numSamples - 1)
                break;
                
            float currentSample = 0.0f;
            float nextSample = 0.0f;
            
            // Average both channels
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                currentSample += buffer.getSample(ch, sampleIndex);
                nextSample += buffer.getSample(ch, sampleIndex + 1);
            }
            currentSample /= buffer.getNumChannels();
            nextSample /= buffer.getNumChannels();
            
            // Check for zero crossing (sign change) and low amplitude
            if (std::abs(currentSample) < threshold && 
                std::abs(nextSample) < threshold &&
                std::signbit(currentSample) != std::signbit(nextSample))
            {
                return sampleIndex;
            }
        }
        
        // If no ideal zero crossing found, find the point closest to zero
        int bestIndex = startSearchFrom;
        float bestAmplitude = std::abs(buffer.getSample(0, startSearchFrom));
        
        for (int i = 0; i < searchRange; ++i)
        {
            int sampleIndex = startSearchFrom + (i * searchDirection);
            if (sampleIndex < 0 || sampleIndex >= numSamples)
                break;
                
            float amplitude = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                amplitude += std::abs(buffer.getSample(ch, sampleIndex));
            }
            amplitude /= buffer.getNumChannels();
            
            if (amplitude < bestAmplitude)
            {
                bestAmplitude = amplitude;
                bestIndex = sampleIndex;
            }
        }
        
        return bestIndex;
    };
    
    // Find optimal start point (search forward from beginning)
    int loopStart = findNearestZeroCrossing(0, 1);
    
    // Find optimal end point (search backward from end)
    int loopEnd = findNearestZeroCrossing(numSamples - 1, -1);
    
    // Ensure we have a reasonable loop length
    if (loopEnd - loopStart < numSamples / 2)
    {
        // If the detected loop is too short, use more of the file
        loopStart = searchRange / 4;
        loopEnd = numSamples - (searchRange / 4);
    }
    
    return {loopStart, loopEnd};
}

NoiseAssetManager::NoiseType NoiseAssetManager::getTypeFromFilename(const juce::String& filename)
{
    auto lower = filename.toLowerCase();
    
    if (lower.startsWith("vinyl"))
        return NoiseType::Vinyl;
    else if (lower.startsWith("tape"))
        return NoiseType::Tape;
    else if (lower.startsWith("hum"))
        return NoiseType::Hum;
    else if (lower.startsWith("fan"))
        return NoiseType::Fan;
    else if (lower.startsWith("jazzclub") || lower.startsWith("jazz"))
        return NoiseType::JazzClub;
    
    return NoiseType::NumTypes; // Invalid
}

//==============================================================================
// ProceduralNoiseGenerator
//==============================================================================

ProceduralNoiseGenerator::ProceduralNoiseGenerator()
{
    reset();
}

void ProceduralNoiseGenerator::prepare(double sampleRate_, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sampleRate = sampleRate_;
    
    // Setup filters for each noise type
    vinylHighpass.setCoeff(sampleRate, 20.0f);
    vinylLowpass.setCoeff(sampleRate, 15000.0f);
    
    tapeHighpass.setCoeff(sampleRate, 30.0f);
    tapeLowpass.setCoeff(sampleRate, 12000.0f);
    
    humBandpass.setCoeff(sampleRate, 60.0f);   // 60Hz bandpass
    humLowpass.setCoeff(sampleRate, 200.0f);   // Gentle lowpass for hum
    
    fanLowpass.setCoeff(sampleRate, 300.0f);   // Fan rumble lowpass
    fanHighpass.setCoeff(sampleRate, 15.0f);   // Remove DC for fan
    
    jazzClubBandpass.setCoeff(sampleRate, 800.0f);
    
    reset();
}

void ProceduralNoiseGenerator::reset()
{
    std::memset(pinkState, 0, sizeof(pinkState));
    pinkCounter = 0;
    cracklePhase = 0.0f;
    nextCrackleTime = fastRand() * 0.5f;
    lfoPhase = 0.0f;
    nextClinkTime = fastRand() * 2.0f;
    
    vinylHighpass.state = 0.0f;
    vinylLowpass.state = 0.0f;
    tapeHighpass.state = 0.0f;
    tapeLowpass.state = 0.0f;
    humBandpass.state = 0.0f;
    humLowpass.state = 0.0f;
    fanLowpass.state = 0.0f;
    fanHighpass.state = 0.0f;
    jazzClubBandpass.state = 0.0f;
}

void ProceduralNoiseGenerator::generateNoise(NoiseAssetManager::NoiseType type,
                                            float* leftOut, float* rightOut,
                                            int numSamples) noexcept
{
    const float timeStep = 1.0f / static_cast<float>(sampleRate);
    
    for (int i = 0; i < numSamples; ++i)
    {
        float left = 0.0f, right = 0.0f;
        
        switch (type)
        {
            case NoiseAssetManager::NoiseType::Vinyl:
            {
                // Pink noise with occasional crackles
                float pink = generatePinkNoise() * 0.25f;  // Increased from 0.1f
                pink = vinylLowpass.process(vinylHighpass.process(pink));
                
                // Add sparse crackles
                cracklePhase += timeStep;
                if (cracklePhase >= nextCrackleTime)
                {
                    float crackle = (fastRand() - 0.5f) * 0.15f;
                    pink += crackle;
                    nextCrackleTime = 0.1f + fastRand() * 0.4f;
                    cracklePhase = 0.0f;
                }
                
                left = pink * (0.9f + fastRand() * 0.1f);
                right = pink * (0.9f + fastRand() * 0.1f);
                break;
            }
            
            case NoiseAssetManager::NoiseType::Tape:
            {
                // Filtered pink noise for tape hiss
                float hiss = generatePinkNoise() * 0.2f;  // Increased from 0.08f
                hiss = tapeLowpass.process(tapeHighpass.process(hiss));
                
                left = hiss * (0.95f + fastRand() * 0.05f);
                right = hiss * (0.95f + fastRand() * 0.05f);
                break;
            }
            
            case NoiseAssetManager::NoiseType::Hum:
            {
                // Enhanced 60Hz electrical hum with realistic harmonics and modulation
                lfoPhase += 60.0f * timeStep;
                if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
                
                // Fundamental and harmonics with proper amplitudes for electrical hum
                float hum = std::sin(2.0f * juce::MathConstants<float>::pi * lfoPhase);              // 60Hz fundamental
                hum += std::sin(4.0f * juce::MathConstants<float>::pi * lfoPhase) * 0.4f;            // 120Hz (strong)
                hum += std::sin(6.0f * juce::MathConstants<float>::pi * lfoPhase) * 0.15f;           // 180Hz
                hum += std::sin(8.0f * juce::MathConstants<float>::pi * lfoPhase) * 0.08f;           // 240Hz
                hum += std::sin(10.0f * juce::MathConstants<float>::pi * lfoPhase) * 0.05f;          // 300Hz
                
                // Add slight amplitude modulation (power supply fluctuation)
                float modPhase = lfoPhase * 0.1f;  // 6Hz modulation
                hum *= (1.0f + std::sin(2.0f * juce::MathConstants<float>::pi * modPhase) * 0.03f);
                
                hum *= 0.04f; // Overall level - increased from 0.015f
                
                // Add filtered broadband noise for transformer buzz
                float buzz = humBandpass.process((fastRand() - 0.5f) * 0.008f);
                buzz = humLowpass.process(buzz);
                
                hum += buzz;
                
                left = right = hum;
                break;
            }
            
            case NoiseAssetManager::NoiseType::Fan:
            {
                // Realistic fan/AC rumble with motor harmonics and air movement
                static float motorPhase = 0.0f;
                motorPhase += 23.0f * timeStep; // ~23Hz motor frequency
                if (motorPhase >= 1.0f) motorPhase -= 1.0f;
                
                // Motor rumble with harmonics
                float motor = std::sin(2.0f * juce::MathConstants<float>::pi * motorPhase);
                motor += std::sin(4.0f * juce::MathConstants<float>::pi * motorPhase) * 0.3f;  // 46Hz
                motor += std::sin(6.0f * juce::MathConstants<float>::pi * motorPhase) * 0.15f; // 69Hz
                motor *= 0.035f;  // Increased from 0.025f for better audibility
                
                // Add slight flutter/wobble to motor
                static float flutterPhase = 0.0f;
                flutterPhase += 1.3f * timeStep; // 1.3Hz flutter
                if (flutterPhase >= 1.0f) flutterPhase -= 1.0f;
                motor *= (1.0f + std::sin(2.0f * juce::MathConstants<float>::pi * flutterPhase) * 0.08f);
                
                // Low-frequency rumble and vibration
                float rumble = fanLowpass.process((fastRand() - 0.5f) * 0.025f);
                rumble = fanHighpass.process(rumble); // Remove DC
                
                // Air movement (filtered white noise)
                float air = fanLowpass.process((fastRand() - 0.5f) * 0.012f);
                
                // Combine components
                float fanNoise = motor + rumble + air;
                
                // Slight stereo variation for realism
                left = fanNoise * (0.95f + fastRand() * 0.05f);
                right = fanNoise * (0.95f + fastRand() * 0.05f);
                break;
            }
            
            
            case NoiseAssetManager::NoiseType::JazzClub:
            {
                // Low crowd murmur with occasional clinks
                float crowd = jazzClubBandpass.process(generatePinkNoise() * 0.025f);  // Increased from 0.008f
                
                // Add occasional clink transients
                nextClinkTime -= timeStep;
                if (nextClinkTime <= 0.0f)
                {
                    float clink = (fastRand() - 0.5f) * 0.08f * std::exp(-cracklePhase * 50.0f);
                    crowd += clink;
                    nextClinkTime = 1.0f + fastRand() * 3.0f;
                    cracklePhase = 0.0f;
                }
                else
                {
                    cracklePhase += timeStep;
                }
                
                left = crowd * (0.8f + fastRand() * 0.2f);
                right = crowd * (0.8f + fastRand() * 0.2f);
                break;
            }
            
            default:
                break;
        }
        
        leftOut[i] = left;
        rightOut[i] = right;
    }
}

float ProceduralNoiseGenerator::generatePinkNoise() noexcept
{
    // Paul Kellet's refined pink noise generator
    float white = (fastRand() - 0.5f) * 2.0f;
    
    pinkState[0] = 0.99886f * pinkState[0] + white * 0.0555179f;
    pinkState[1] = 0.99332f * pinkState[1] + white * 0.0750759f;
    pinkState[2] = 0.96900f * pinkState[2] + white * 0.1538520f;
    pinkState[3] = 0.86650f * pinkState[3] + white * 0.3104856f;
    pinkState[4] = 0.55000f * pinkState[4] + white * 0.5329522f;
    pinkState[5] = -0.7616f * pinkState[5] - white * 0.0168980f;
    
    float pink = pinkState[0] + pinkState[1] + pinkState[2] + pinkState[3] 
               + pinkState[4] + pinkState[5] + pinkState[6] + white * 0.5362f;
    pinkState[6] = white * 0.115926f;
    
    return pink * 0.11f; // Scale to roughly -1 to 1
}

void ProceduralNoiseGenerator::SimpleFilter::setCoeff(double sampleRate, float freq)
{
    coeff = std::exp(-2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate));
}

float ProceduralNoiseGenerator::SimpleFilter::process(float input) noexcept
{
    state = input + coeff * (state - input);
    return state;
}

}