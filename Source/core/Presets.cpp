#include "Presets.h"
#include "Params.h"

namespace ReallyCheap
{

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts_)
    : apvts(apvts_)
{
    initializeFactoryPresets();
}

bool PresetManager::savePreset(const juce::String& presetName, const juce::File& presetFile)
{
    juce::ignoreUnused(presetName);  // Name is stored in filename
    auto currentState = getPresetValueTree();
    return saveJsonToFile(currentState, presetFile);
}

bool PresetManager::loadPreset(const juce::File& presetFile, bool respectLocks)
{
    if (!presetFile.exists())
        return false;
        
    auto presetState = loadJsonFromFile(presetFile);
    if (!presetState.isValid())
        return false;
    
    // Apply synchronously - we're already on the message thread when called from UI
    applyValueTreeToAPVTS(presetState, respectLocks);
    
    return true;
}

juce::ValueTree PresetManager::getPresetValueTree() const
{
    juce::ValueTree presetState("Parameters");
    
    // Save actual parameter values (not normalized) for better JSON readability
    for (auto* param : apvts.processor.getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
        {
            auto paramID = paramWithID->paramID;
            
            // Get the actual parameter value (not normalized)
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(paramWithID))
            {
                auto actualValue = rangedParam->convertFrom0to1(paramWithID->getValue());
                presetState.setProperty(paramID, actualValue, nullptr);
            }
            else
            {
                // Fallback for parameters without range info
                presetState.setProperty(paramID, paramWithID->getValue(), nullptr);
            }
        }
    }
    
    return presetState;
}

void PresetManager::setPresetValueTree(const juce::ValueTree& presetState, bool respectLocks)
{
    applyValueTreeToAPVTS(presetState, respectLocks);
}

bool PresetManager::loadFactoryPreset(const juce::String& presetName)
{
    auto factoryDir = getFactoryPresetsDirectory();
    
    // Debug: Print directory info
    DBG("Factory preset directory: " + factoryDir.getFullPathName());
    DBG("Directory exists: " + juce::String(factoryDir.exists() ? "YES" : "NO"));
    
    auto presetFile = factoryDir.getChildFile(presetName + ".rc20preset");
    DBG("Looking for preset file: " + presetFile.getFullPathName());
    DBG("Preset file exists: " + juce::String(presetFile.exists() ? "YES" : "NO"));
    
    if (!presetFile.exists())
    {
        // Try to find by filename without extension
        DBG("Searching all files in factory directory...");
        auto files = factoryDir.findChildFiles(juce::File::findFiles, false, "*.rc20preset");
        DBG("Found " + juce::String(files.size()) + " preset files");
        
        for (auto file : files)
        {
            DBG("Found file: " + file.getFullPathName() + " (name: " + file.getFileNameWithoutExtension() + ")");
            if (file.getFileNameWithoutExtension() == presetName)
            {
                presetFile = file;
                DBG("Match found!");
                break;
            }
        }
    }
    
    if (!presetFile.exists())
    {
        DBG("ERROR: Could not find preset file for: " + presetName);
        return false;
    }
    
    DBG("Loading preset from: " + presetFile.getFullPathName());
    return loadPreset(presetFile, false);
}

juce::StringArray PresetManager::getFactoryPresetNames() const
{
    return factoryPresetNames;
}

juce::File PresetManager::getFactoryPresetsDirectory() const
{
    // Try multiple possible locations for factory presets
    juce::Array<juce::File> searchPaths;
    
    // 1. Relative to plugin location
    auto pluginDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory();
    searchPaths.add(pluginDir.getChildFile("presets").getChildFile("factory"));
    
    // 2. Project directory (for development)
    searchPaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("presets").getChildFile("factory"));
    
    // 3. User's Documents folder
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    searchPaths.add(documentsDir.getChildFile("ReallyCheap").getChildFile("Twenty").getChildFile("Factory Presets"));
    
    // 4. Next to VST3 file (common location)
    searchPaths.add(juce::File("C:\\Program Files\\Common Files\\VST3\\ReallyCheap-Twenty.vst3").getParentDirectory().getChildFile("presets").getChildFile("factory"));
    
    // 5. Development path (absolute)
    searchPaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\presets\\factory"));
    
    for (auto& path : searchPaths)
    {
        DBG("Checking factory preset path: " + path.getFullPathName());
        if (path.exists() && path.isDirectory())
        {
            DBG("Found factory presets at: " + path.getFullPathName());
            return path;
        }
    }
    
    // Return the first path as default (even if it doesn't exist)
    DBG("No factory preset directory found, using default: " + searchPaths[0].getFullPathName());
    return searchPaths[0];
}

bool PresetManager::saveUserPreset(const juce::String& presetName)
{
    auto userDir = getUserPresetsDirectory();
    userDir.createDirectory();
    
    auto sanitizedName = sanitizePresetName(presetName);
    auto presetFile = userDir.getChildFile(sanitizedName + ".rc20preset");
    
    return savePreset(sanitizedName, presetFile);
}

bool PresetManager::loadUserPreset(const juce::String& presetName)
{
    auto userDir = getUserPresetsDirectory();
    auto presetFile = userDir.getChildFile(presetName + ".rc20preset");
    
    return loadPreset(presetFile, false);
}

bool PresetManager::deleteUserPreset(const juce::String& presetName)
{
    auto userDir = getUserPresetsDirectory();
    auto presetFile = userDir.getChildFile(presetName + ".rc20preset");
    
    if (presetFile.exists())
        return presetFile.deleteFile();
        
    return false;
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    juce::StringArray userPresets;
    auto userDir = getUserPresetsDirectory();
    
    if (userDir.exists())
    {
        for (auto file : userDir.findChildFiles(juce::File::findFiles, false, "*.rc20preset"))
        {
            userPresets.add(file.getFileNameWithoutExtension());
        }
    }
    
    return userPresets;
}

juce::File PresetManager::getUserPresetsDirectory() const
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    return appDataDir.getChildFile("ReallyCheap").getChildFile("Twenty").getChildFile("Presets");
}

void PresetManager::setCurrentPresetIndex(int index)
{
    auto allPresets = getAllPresetNames();
    currentPresetIndex = juce::jlimit(0, allPresets.size() - 1, index);
}

bool PresetManager::loadNextPreset()
{
    auto allPresets = getAllPresetNames();
    if (allPresets.isEmpty())
        return false;
        
    currentPresetIndex = (currentPresetIndex + 1) % allPresets.size();
    
    auto presetName = allPresets[currentPresetIndex];
    
    // Check if it's a factory preset
    if (factoryPresetNames.contains(presetName))
        return loadFactoryPreset(presetName);
    else
        return loadUserPreset(presetName);
}

bool PresetManager::loadPreviousPreset()
{
    auto allPresets = getAllPresetNames();
    if (allPresets.isEmpty())
        return false;
        
    currentPresetIndex = (currentPresetIndex - 1 + allPresets.size()) % allPresets.size();
    
    auto presetName = allPresets[currentPresetIndex];
    
    // Check if it's a factory preset
    if (factoryPresetNames.contains(presetName))
        return loadFactoryPreset(presetName);
    else
        return loadUserPreset(presetName);
}

juce::StringArray PresetManager::getAllPresetNames() const
{
    juce::StringArray allPresets;
    allPresets.addArray(getFactoryPresetNames());
    allPresets.addArray(getUserPresetNames());
    return allPresets;
}

int PresetManager::getTotalPresetCount() const
{
    return getAllPresetNames().size();
}

void PresetManager::setModuleLocked(const juce::String& moduleId, bool locked)
{
    moduleLocks[moduleId] = locked;
}

bool PresetManager::isModuleLocked(const juce::String& moduleId) const
{
    auto it = moduleLocks.find(moduleId);
    return it != moduleLocks.end() ? it->second : false;
}

void PresetManager::clearAllLocks()
{
    moduleLocks.clear();
}

juce::String PresetManager::sanitizePresetName(const juce::String& name)
{
    juce::String sanitized = name;
    
    // Remove/replace invalid filename characters
    sanitized = sanitized.replaceCharacters("<>:\"/\\|?*", "________");
    
    // Trim whitespace
    sanitized = sanitized.trim();
    
    // Ensure it's not empty
    if (sanitized.isEmpty())
        sanitized = "Untitled";
        
    return sanitized;
}

void PresetManager::loadPresetAsync(const juce::File& presetFile, std::function<void(bool)> callback)
{
    // Perform file I/O on background thread
    juce::Thread::launch([this, presetFile, callback]()
    {        
        if (presetFile.exists())
        {
            auto presetState = loadJsonFromFile(presetFile);
            if (presetState.isValid())
            {
                // Apply on message thread
                juce::MessageManager::callAsync([this, presetState, callback]()
                {
                    applyValueTreeToAPVTS(presetState, false);
                    if (callback)
                        callback(true);
                });
                return;
            }
        }
        
        // Failed - callback on message thread
        if (callback)
        {
            juce::MessageManager::callAsync([callback]()
            {
                callback(false);
            });
        }
    });
}

// Private helper methods

juce::ValueTree PresetManager::valueTreeFromJson(const juce::String& jsonString)
{
    // Use XML as intermediate format since JSON serialization is complex
    auto xmlElement = juce::XmlDocument::parse(jsonString);
    if (xmlElement != nullptr)
        return juce::ValueTree::fromXml(*xmlElement);
        
    // Fallback: try direct JSON parsing for simple parameter values
    auto parseResult = juce::JSON::parse(jsonString);
    if (parseResult.isVoid())
        return {};
        
    // Create a simple ValueTree from JSON parameters
    juce::ValueTree tree("Parameters");
    if (auto* obj = parseResult.getDynamicObject())
    {
        for (auto& prop : obj->getProperties())
        {
            tree.setProperty(prop.name, prop.value, nullptr);
        }
    }
    return tree;
}

juce::String PresetManager::valueTreeToJson(const juce::ValueTree& valueTree)
{
    // Convert ValueTree parameters to JSON object
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    
    for (int i = 0; i < valueTree.getNumProperties(); ++i)
    {
        auto propertyName = valueTree.getPropertyName(i);
        auto propertyValue = valueTree.getProperty(propertyName);
        obj->setProperty(propertyName, propertyValue);
    }
    
    return juce::JSON::toString(obj.get());
}

bool PresetManager::saveJsonToFile(const juce::ValueTree& valueTree, const juce::File& file)
{
    auto jsonString = valueTreeToJson(valueTree);
    
    if (jsonString.isEmpty())
        return false;
        
    // Ensure parent directory exists
    file.getParentDirectory().createDirectory();
    
    return file.replaceWithText(jsonString);
}

juce::ValueTree PresetManager::loadJsonFromFile(const juce::File& file)
{
    if (!file.exists())
    {
        DBG("ERROR: Preset file does not exist: " + file.getFullPathName());
        return {};
    }
        
    auto jsonString = file.loadFileAsString();
    DBG("Loaded JSON string (" + juce::String(jsonString.length()) + " chars): " + jsonString.substring(0, 200) + "...");
    
    auto valueTree = valueTreeFromJson(jsonString);
    DBG("Converted to ValueTree with " + juce::String(valueTree.getNumProperties()) + " properties");
    
    return valueTree;
}

void PresetManager::applyValueTreeToAPVTS(const juce::ValueTree& valueTree, bool respectLocks)
{
    if (!valueTree.isValid())
    {
        DBG("ERROR: Invalid ValueTree passed to applyValueTreeToAPVTS");
        return;
    }
    
    DBG("Applying preset with " + juce::String(valueTree.getNumProperties()) + " properties");
    
    int parametersSet = 0;
    
    // Apply parameter values, respecting locks if specified
    for (auto* param : apvts.processor.getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
        {
            auto paramID = paramWithID->paramID;
            
            // Check if this parameter's module is locked
            if (respectLocks)
            {
                // Simple lock check based on parameter ID prefix
                // This is a scaffold - could be more sophisticated
                if (paramID.startsWith("distort") && isModuleLocked("distort"))
                    continue;
                if (paramID.startsWith("noise") && isModuleLocked("noise"))
                    continue;
                if (paramID.startsWith("wobble") && isModuleLocked("wobble"))
                    continue;
                if (paramID.startsWith("digital") && isModuleLocked("digital"))
                    continue;
                if (paramID.startsWith("space") && isModuleLocked("space"))
                    continue;
                if (paramID.startsWith("mag") && isModuleLocked("mag"))
                    continue;
            }
            
            // Apply parameter value if it exists in preset
            if (valueTree.hasProperty(paramID))
            {
                auto value = valueTree.getProperty(paramID);
                DBG("Setting parameter " + paramID + " to " + value.toString());
                
                // Convert the actual parameter value to normalized value (0.0-1.0)
                if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(paramWithID))
                {
                    auto normalizedValue = rangedParam->convertTo0to1(value);
                    DBG("  Normalized value: " + juce::String(normalizedValue));
                    paramWithID->setValueNotifyingHost(normalizedValue);
                    parametersSet++;
                }
                else
                {
                    // Fallback for parameters without range info
                    DBG("  Using raw value (no range conversion)");
                    paramWithID->setValueNotifyingHost(value);
                    parametersSet++;
                }
            }
            else
            {
                DBG("Parameter " + paramID + " not found in preset");
            }
        }
    }
    
    DBG("Applied " + juce::String(parametersSet) + " parameters from preset");
}

void PresetManager::initializeFactoryPresets()
{
    factoryPresetNames = {
        "Subtle Glue",
        "Dusty Tape",
        "Warped Cassette", 
        "VHS Yearbook",
        "Mall PA",
        "Cheap Spring",
        "Vinyl Floor",
        "Broadcast"
    };
}

}