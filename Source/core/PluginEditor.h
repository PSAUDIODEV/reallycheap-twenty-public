#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../ui/LookAndFeel.h"  
#include "../ui/ModulePanels/DistortPanel.h"
#include "../ui/ModulePanels/WobblePanel.h"
#include "../ui/ModulePanels/DigitalPanel.h"
#include "../ui/ModulePanels/MagneticPanel.h"
#include "../ui/ModulePanels/NoisePanel.h"
#include "../ui/ModulePanels/SpacePanel.h"

class ReallyCheapTwentyAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ReallyCheapTwentyAudioProcessorEditor(ReallyCheapTwentyAudioProcessor&);
    ~ReallyCheapTwentyAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ReallyCheapTwentyAudioProcessor& audioProcessor;
    
    std::unique_ptr<ReallyCheap::ReallyCheapLookAndFeel> lookAndFeel;
    
    // Module panels
    std::unique_ptr<ReallyCheap::DistortPanel> distortPanel;
    std::unique_ptr<ReallyCheap::WobblePanel> wobblePanel;
    std::unique_ptr<ReallyCheap::DigitalPanel> digitalPanel;
    std::unique_ptr<ReallyCheap::MagneticPanel> magneticPanel;
    std::unique_ptr<NoisePanel> noisePanel;
    std::unique_ptr<ReallyCheap::SpacePanel> spacePanel;
    
    // Global controls
    juce::Slider inGainSlider;
    juce::Slider outGainSlider;
    juce::Slider mixSlider;
    juce::ToggleButton bypassButton;
    juce::Slider macroSlider;
    
    juce::Label inGainLabel;
    juce::Label outGainLabel;
    juce::Label mixLabel;
    juce::Label bypassLabel;
    juce::Label macroLabel;
    juce::Label titleLabel;
    
    // Background SVG from Figma
    std::unique_ptr<juce::Drawable> backgroundSVG;
    
    // Custom SVG knobs - Main
    std::unique_ptr<juce::Drawable> inputGainKnobSVG;
    std::unique_ptr<juce::Drawable> outputGainKnobSVG;
    std::unique_ptr<juce::Drawable> mixKnobSVG;
    std::unique_ptr<juce::Drawable> macroKnobSVG;
    
    // Bend module knobs (formerly Wobble)
    juce::Slider wobbleDepthSlider;
    juce::Slider wobbleRateSlider;
    juce::Slider wobbleFlutterSlider;
    juce::Slider wobbleDriftSlider;
    juce::Slider wobbleJitterSlider;
    juce::Slider wobbleLinkSlider;
    
    std::unique_ptr<juce::Drawable> wobbleDepthKnobSVG;
    std::unique_ptr<juce::Drawable> wobbleRateKnobSVG;
    std::unique_ptr<juce::Drawable> wobbleFlutterKnobSVG;
    std::unique_ptr<juce::Drawable> wobbleDriftKnobSVG;
    std::unique_ptr<juce::Drawable> wobbleJitterKnobSVG;
    std::unique_ptr<juce::Drawable> wobbleLinkKnobSVG;
    
    // Bend module switches (formerly Wobble)
    juce::ToggleButton wobbleOnButton;
    juce::ToggleButton wobbleSyncButton;
    juce::ToggleButton wobbleMonoButton;
    
    std::unique_ptr<juce::Drawable> wobbleOnSwitchSVG;
    std::unique_ptr<juce::Drawable> wobbleSyncSwitchSVG;
    std::unique_ptr<juce::Drawable> wobbleMonoSwitchSVG;
    std::unique_ptr<juce::Drawable> switchOffSVG;
    
    // Title card PNG
    juce::Image titleCardImage;
    
    // Preset selector SVGs
    std::unique_ptr<juce::Drawable> presetSelectorClosedSVG;
    std::unique_ptr<juce::Drawable> presetSelectorOpenSVG;
    
    // Custom font
    juce::Font customFont{juce::FontOptions(12.0f)};

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroAttachment;
    
    // Bend module attachments (formerly Wobble)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wobbleDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wobbleRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wobbleFlutterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wobbleDriftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wobbleJitterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wobbleLinkAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> wobbleOnAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> wobbleSyncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> wobbleMonoAttachment;
    
    // Bitcrush module knobs (formerly Digital)
    juce::Slider digitalBitsSlider;
    juce::Slider digitalSRSlider;
    juce::Slider digitalJitterSlider;
    
    std::unique_ptr<juce::Drawable> digitalBitsKnobSVG;
    std::unique_ptr<juce::Drawable> digitalSRKnobSVG;
    std::unique_ptr<juce::Drawable> digitalJitterKnobSVG;
    
    // Bitcrush module switches (formerly Digital)
    juce::ToggleButton digitalOnButton;
    juce::ToggleButton digitalAAButton;
    
    std::unique_ptr<juce::Drawable> digitalOnSwitchSVG;
    std::unique_ptr<juce::Drawable> digitalAASwitchSVG;
    
    // Bitcrush module attachments (formerly Digital)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> digitalBitsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> digitalSRAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> digitalJitterAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> digitalOnAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> digitalAAAttachment;
    
    // Crunch module knobs (formerly Distortion)
    juce::Slider distortDriveSlider;
    juce::Slider distortToneSlider;
    
    std::unique_ptr<juce::Drawable> distortDriveKnobSVG;
    std::unique_ptr<juce::Drawable> distortToneKnobSVG;
    
    // Crunch module switch and selector (formerly Distortion)
    juce::ToggleButton distortOnButton;
    juce::ToggleButton distortPrePostButton;
    juce::ComboBox distortTypeSelector;
    
    std::unique_ptr<juce::Drawable> distortOnSwitchSVG;
    std::unique_ptr<juce::Drawable> distortPrePostSwitchSVG;
    std::unique_ptr<juce::Drawable> distortTypeSelectorSVG;
    
    // Crunch module attachments (formerly Distortion)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distortDriveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distortToneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> distortOnAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> distortPrePostAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distortTypeAttachment;
    
    // Tape module knobs (formerly Magnetic)
    juce::Slider magneticCompSlider;
    juce::Slider magneticSatSlider;
    juce::Slider magneticXTalkSlider;
    juce::Slider magneticHeadSlider;
    juce::Slider magneticWearSlider;
    
    std::unique_ptr<juce::Drawable> magneticCompKnobSVG;
    std::unique_ptr<juce::Drawable> magneticSatKnobSVG;
    std::unique_ptr<juce::Drawable> magneticXTalkKnobSVG;
    std::unique_ptr<juce::Drawable> magneticHeadKnobSVG;
    std::unique_ptr<juce::Drawable> magneticWearKnobSVG;
    
    // Tape module switch (formerly Magnetic)
    juce::ToggleButton magneticOnButton;
    
    std::unique_ptr<juce::Drawable> magneticOnSwitchSVG;
    
    // Tape module attachments (formerly Magnetic)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> magneticCompAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> magneticSatAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> magneticXTalkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> magneticHeadAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> magneticWearAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> magneticOnAttachment;

    // Atmosphere module knobs (formerly Noise)
    juce::Slider noiseLevelSlider;
    juce::Slider noiseAgeSlider;
    juce::Slider noiseFlutterSlider;
    juce::Slider noiseWidthSlider;
    
    std::unique_ptr<juce::Drawable> noiseLevelKnobSVG;
    std::unique_ptr<juce::Drawable> noiseAgeKnobSVG;
    std::unique_ptr<juce::Drawable> noiseFlutterKnobSVG;
    std::unique_ptr<juce::Drawable> noiseWidthKnobSVG;
    
    // Atmosphere module switches and selector (formerly Noise)
    juce::ToggleButton noiseOnButton;
    juce::ToggleButton noisePrePostButton;
    juce::ComboBox noiseTypeSelector;
    juce::Label noiseTypeLabel;
    
    std::unique_ptr<juce::Drawable> noiseOnSwitchSVG;
    std::unique_ptr<juce::Drawable> noisePrePostSwitchSVG;
    std::unique_ptr<juce::Drawable> noiseTypeSelectorSVG;
    
    // Atmosphere module attachments (formerly Noise)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseAgeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseFlutterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseWidthAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> noiseOnAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> noisePrePostAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> noiseTypeAttachment;

    // Verb module knobs (formerly Space)
    juce::Slider spaceMixSlider;
    juce::Slider spaceTimeSlider;
    juce::Slider spaceToneSlider;
    juce::Slider spacePreDelaySlider;
    juce::Slider spaceCheapoSlider;
    
    std::unique_ptr<juce::Drawable> spaceMixKnobSVG;
    std::unique_ptr<juce::Drawable> spaceTimeKnobSVG;
    std::unique_ptr<juce::Drawable> spaceToneKnobSVG;
    std::unique_ptr<juce::Drawable> spacePreDelayKnobSVG;
    std::unique_ptr<juce::Drawable> spaceCheapoKnobSVG;
    
    // Verb module switch (formerly Space)
    juce::ToggleButton spaceOnButton;
    
    std::unique_ptr<juce::Drawable> spaceOnSwitchSVG;
    
    // Verb module attachments (formerly Space)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spaceMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spaceTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spaceToneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spacePreDelayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spaceCheapoAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> spaceOnAttachment;

    // Preset UI controls
    juce::ComboBox presetComboBox;
    juce::TextButton saveButton;
    juce::TextButton loadButton;
    juce::TextButton prevButton;
    juce::TextButton nextButton;
    juce::Label presetLabel;

    float currentUIScale = 0.5f;  // Start at 50% scale (current size)
    
    // Resize handle
    std::unique_ptr<juce::ResizableCornerComponent> resizer;
    juce::ComponentBoundsConstrainer resizeConstrainer;
    
    void updateUIScale();
    void updateUIScale(float newScale);
    void setupMainKnobs();
    void setupWobbleKnobs();        // Bend module (formerly Wobble)
    void setupWobbleSwitches();     // Bend module switches
    void setupDigitalKnobs();       // Bitcrush module (formerly Digital)
    void setupDigitalSwitches();    // Bitcrush module switches
    void setupDistortKnobs();       // Crunch module (formerly Distortion)
    void setupDistortControls();    // Crunch module controls
    void setupMagneticKnobs();      // Tape module (formerly Magnetic)
    void setupMagneticControls();   // Tape module controls
    void setupNoiseKnobs();         // Atmosphere module (formerly Noise)
    void setupNoiseControls();      // Atmosphere module controls
    void setupSpaceKnobs();         // Verb module (formerly Space)
    void setupSpaceControls();      // Verb module controls
    void setupGlobalControls();
    void setupPresetControls();
    void updatePresetComboBox();
    void loadBackgroundSVG();
    void loadKnobSVGs();
    void loadWobbleKnobSVGs();       // Bend module SVGs
    void loadWobbleSwitchSVGs();     // Bend module switch SVGs
    void loadDigitalKnobSVGs();      // Bitcrush module SVGs
    void loadDigitalSwitchSVGs();    // Bitcrush module switch SVGs
    void loadDistortKnobSVGs();      // Crunch module SVGs
    void loadDistortControlSVGs();   // Crunch module control SVGs
    void loadMagneticKnobSVGs();     // Tape module SVGs
    void loadMagneticControlSVGs();  // Tape module control SVGs
    void loadNoiseKnobSVGs();        // Atmosphere module SVGs
    void loadNoiseControlSVGs();     // Atmosphere module control SVGs
    void loadSpaceKnobSVGs();        // Verb module SVGs
    void loadSpaceControlSVGs();     // Verb module control SVGs
    void loadTitleCardImage();
    void loadPresetSelectorSVGs();
    void loadCustomFont();
    void drawPresetSelector(juce::Graphics& g);
    void drawWobbleKnobs(juce::Graphics& g);    // Bend module drawing
    void drawWobbleSwitches(juce::Graphics& g); // Bend module switches
    void drawDigitalKnobs(juce::Graphics& g);   // Bitcrush module drawing
    void drawDigitalSwitches(juce::Graphics& g);// Bitcrush module switches
    void drawDistortKnobs(juce::Graphics& g);   // Crunch module drawing
    void drawDistortControls(juce::Graphics& g);// Crunch module controls
    void drawMagneticKnobs(juce::Graphics& g);  // Tape module drawing
    void drawMagneticControls(juce::Graphics& g);// Tape module controls
    void drawNoiseKnobs(juce::Graphics& g);     // Atmosphere module drawing
    void drawNoiseControls(juce::Graphics& g);  // Atmosphere module controls
    void drawSpaceKnobs(juce::Graphics& g);     // Verb module drawing
    void drawSpaceControls(juce::Graphics& g);  // Verb module controls
    
    // Preset control callbacks
    void savePresetClicked();
    void loadPresetClicked();
    void loadSelectedPreset();
    void prevPresetClicked();
    void nextPresetClicked();
    void presetComboChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReallyCheapTwentyAudioProcessorEditor)
};