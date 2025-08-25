#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include <vector>

namespace ReallyCheap
{

class NoiseAssetManager
{
public:
    enum class NoiseType
    {
        Vinyl = 0,      // Asset-based - vinyl crackle/surface noise
        Tape,           // Asset-based - tape hiss 
        Hum,            // Procedural - 60Hz electrical hum
        Fan,            // Procedural - fan/air conditioning rumble
        JazzClub,       // Asset-based - ambient club atmosphere
        NumTypes
    };
    
    struct AssetBuffer
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 44100.0;
        juce::String filename;
        int loopStartSample = 0;    // Optimal loop start point (zero crossing)
        int loopEndSample = 0;      // Optimal loop end point (zero crossing)
    };
    
    // Singleton access
    static NoiseAssetManager& getInstance();
    
    // Load assets from folder (message thread only)
    void loadAssetsFromFolder(const juce::File& folder);
    
    // Load assets from binary data (preferred method)
    void loadAssetsFromBinaryData();
    
    // RT-safe getters - returns nullptr if no assets for type
    const AssetBuffer* getAssetForType(NoiseType type) const noexcept;
    
    // Get number of assets for a type
    size_t getAssetCountForType(NoiseType type) const noexcept;
    
    // Check if procedural fallback is needed
    bool needsProceduralFallback(NoiseType type) const noexcept;
    
    // Check if a noise type should always use procedural generation
    static bool isProceduralType(NoiseType type) noexcept;
    
private:
    NoiseAssetManager() = default;
    ~NoiseAssetManager() = default;
    
    // Per-type asset storage (atomic for RT-safe swapping)
    struct AssetCollection
    {
        std::vector<std::unique_ptr<AssetBuffer>> buffers;
        std::atomic<size_t> currentIndex{0};
    };
    
    // Double-buffered for RT-safe updates
    std::unique_ptr<AssetCollection[]> activeAssets;
    std::unique_ptr<AssetCollection[]> pendingAssets;
    std::atomic<AssetCollection*> assetPointer{nullptr};
    
    // Load a single audio file (WAV or MP3) with zero-crossing detection
    std::unique_ptr<AssetBuffer> loadAudioFile(const juce::File& file);
    
    // Load audio from memory (binary data)
    std::unique_ptr<AssetBuffer> loadAudioFromMemory(const char* data, int size, const juce::String& filename);
    
    // Internal file loading (for fallback)
    void loadAssetsFromFolderInternal(const juce::File& folder);
    
    // Find optimal zero-crossing loop points
    std::pair<int, int> findZeroCrossingLoopPoints(const juce::AudioBuffer<float>& buffer);
    
    // Get type from filename
    static NoiseType getTypeFromFilename(const juce::String& filename);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseAssetManager)
};

// Procedural noise generators (fallback when assets missing)
class ProceduralNoiseGenerator
{
public:
    ProceduralNoiseGenerator();
    
    void prepare(double sampleRate, int blockSize);
    void reset();
    
    // Generate procedural noise for given type
    void generateNoise(NoiseAssetManager::NoiseType type,
                       float* leftOut, float* rightOut,
                       int numSamples) noexcept;
    
private:
    double sampleRate = 44100.0;
    
    // Pink noise state
    float pinkState[16] = {};
    int pinkCounter = 0;
    
    // Crackle/tick generator state
    float cracklePhase = 0.0f;
    float nextCrackleTime = 0.0f;
    
    // LFO for hum/fan
    float lfoPhase = 0.0f;
    
    // Clink transient state for jazz club
    float nextClinkTime = 0.0f;
    
    // Fast random number generator
    uint32_t randSeed = 12345;
    float fastRand() noexcept
    {
        randSeed = randSeed * 1103515245 + 12345;
        return (randSeed & 0x7fffffff) / 2147483648.0f;
    }
    
    // Generate pink noise sample
    float generatePinkNoise() noexcept;
    
    // Generate filtered white noise
    float generateFilteredWhite(float cutoff) noexcept;
    
    // Simple one-pole filters for shaping
    struct SimpleFilter
    {
        float state = 0.0f;
        void setCoeff(double sampleRate, float freq);
        float process(float input) noexcept;
        float coeff = 0.0f;
    };
    
    SimpleFilter vinylHighpass, vinylLowpass;
    SimpleFilter tapeHighpass, tapeLowpass;
    SimpleFilter humBandpass, humLowpass;
    SimpleFilter fanLowpass, fanHighpass;
    SimpleFilter jazzClubBandpass;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProceduralNoiseGenerator)
};

}