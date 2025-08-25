#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include <vector>

namespace ReallyCheap
{

class SpaceIRManager
{
public:
    struct IRBuffer
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 44100.0;
        juce::String filename;
        juce::String category; // "spring" or "room"
        float lengthSeconds = 0.0f;
    };
    
    // Singleton access
    static SpaceIRManager& getInstance();
    
    // Load IRs from folder (message thread only)
    void loadIRsFromFolder(const juce::File& folder);
    
    // RT-safe getters
    const IRBuffer* getIR(int index) const noexcept;
    const IRBuffer* getSpringIR(int variant = 0) const noexcept;
    const IRBuffer* getRoomIR(int variant = 0) const noexcept;
    const IRBuffer* getBestIRForTime(float timeSeconds) const noexcept;
    
    // Get counts
    size_t getTotalIRCount() const noexcept;
    size_t getSpringIRCount() const noexcept;
    size_t getRoomIRCount() const noexcept;
    
    // Check if fallback is needed
    bool needsFallbackIR() const noexcept;
    
    // Generate fallback IR (message thread)
    static std::unique_ptr<IRBuffer> generateFallbackIR(double sampleRate, float lengthSeconds);
    
private:
    SpaceIRManager() = default;
    ~SpaceIRManager() = default;
    
    // IR storage (atomic for RT-safe swapping)
    struct IRCollection
    {
        std::vector<std::unique_ptr<IRBuffer>> allIRs;
        std::vector<IRBuffer*> springIRs;
        std::vector<IRBuffer*> roomIRs;
    };
    
    // Double-buffered for RT-safe updates
    std::unique_ptr<IRCollection> activeIRs;
    std::unique_ptr<IRCollection> pendingIRs;
    std::atomic<IRCollection*> irPointer{nullptr};
    
    // Fallback IR (always available)
    std::unique_ptr<IRBuffer> fallbackIR;
    
    // Load a single WAV file
    std::unique_ptr<IRBuffer> loadIRFile(const juce::File& file);
    
    // Parse category from filename
    static juce::String getCategoryFromFilename(const juce::String& filename);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpaceIRManager)
};

// Fallback IR generator
class FallbackIRGenerator
{
public:
    static void generateDecayingNoise(juce::AudioBuffer<float>& buffer, 
                                     double sampleRate, 
                                     float lengthSeconds,
                                     float decayTime = 0.3f) noexcept;
    
    static void applySpringColoration(juce::AudioBuffer<float>& buffer, 
                                     double sampleRate) noexcept;
    
    static void applyRoomColoration(juce::AudioBuffer<float>& buffer, 
                                   double sampleRate) noexcept;
    
private:
    // Simple filters for IR shaping
    struct SimpleFilter
    {
        float state = 0.0f;
        float coeff = 0.0f;
        
        void setLowpass(double sampleRate, float freq);
        void setHighpass(double sampleRate, float freq);
        float process(float input) noexcept;
    };
    
    static uint32_t fastRand(uint32_t& seed) noexcept;
};

}