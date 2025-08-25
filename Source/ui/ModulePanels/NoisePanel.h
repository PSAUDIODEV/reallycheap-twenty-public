#pragma once

#include <JuceHeader.h>

class NoisePanel : public juce::Component
{
public:
    NoisePanel(juce::AudioProcessorValueTreeState& apvts);
    ~NoisePanel() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    // Controls
    juce::ToggleButton onButton;
    juce::ComboBox typeCombo;
    juce::Slider levelSlider;
    juce::Slider ageSlider;
    juce::Slider flutterGateSlider;
    juce::Slider widthSlider;
    juce::ComboBox placementCombo;
    
    // Labels
    juce::Label titleLabel;
    juce::Label typeLabel;
    juce::Label levelLabel;
    juce::Label ageLabel;
    juce::Label flutterGateLabel;
    juce::Label widthLabel;
    juce::Label placementLabel;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ageAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flutterGateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> placementAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoisePanel)
};