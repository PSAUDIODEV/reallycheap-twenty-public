#pragma once

#include <JuceHeader.h>

namespace ReallyCheap
{

class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager() = default;
    
    // Core preset operations
    bool savePreset(const juce::String& presetName, const juce::File& presetFile);
    bool loadPreset(const juce::File& presetFile, bool respectLocks = false);
    juce::ValueTree getPresetValueTree() const;
    void setPresetValueTree(const juce::ValueTree& presetState, bool respectLocks = false);
    
    // Factory presets
    bool loadFactoryPreset(const juce::String& presetName);
    juce::StringArray getFactoryPresetNames() const;
    juce::File getFactoryPresetsDirectory() const;
    
    // User presets
    bool saveUserPreset(const juce::String& presetName);
    bool loadUserPreset(const juce::String& presetName);
    bool deleteUserPreset(const juce::String& presetName);
    juce::StringArray getUserPresetNames() const;
    juce::File getUserPresetsDirectory() const;
    
    // Preset navigation
    void setCurrentPresetIndex(int index);
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    bool loadNextPreset();
    bool loadPreviousPreset();
    
    // Combined preset list (factory + user)
    juce::StringArray getAllPresetNames() const;
    int getTotalPresetCount() const;
    
    // Lock functionality (for future use)
    void setModuleLocked(const juce::String& moduleId, bool locked);
    bool isModuleLocked(const juce::String& moduleId) const;
    void clearAllLocks();
    
    // Utility
    static juce::String sanitizePresetName(const juce::String& name);
    
    // Thread safety - async loading for user presets
    void loadPresetAsync(const juce::File& presetFile, std::function<void(bool)> callback);
    
private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::StringArray factoryPresetNames;
    int currentPresetIndex = 0;
    
    // Lock state
    std::map<juce::String, bool> moduleLocks;
    
    // Helper methods
    juce::ValueTree valueTreeFromJson(const juce::String& jsonString);
    juce::String valueTreeToJson(const juce::ValueTree& valueTree);
    bool saveJsonToFile(const juce::ValueTree& valueTree, const juce::File& file);
    juce::ValueTree loadJsonFromFile(const juce::File& file);
    void applyValueTreeToAPVTS(const juce::ValueTree& valueTree, bool respectLocks);
    void initializeFactoryPresets();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

}