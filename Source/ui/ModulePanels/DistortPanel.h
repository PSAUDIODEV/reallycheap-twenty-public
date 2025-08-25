#pragma once

#include <JuceHeader.h>
#include "../../core/Params.h"
#include "../LookAndFeel.h"

namespace ReallyCheap
{

class DistortPanel : public juce::Component
{
public:
    DistortPanel(juce::AudioProcessorValueTreeState& apvts);
    ~DistortPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    
    // Basic distortion controls
    juce::ToggleButton onButton;
    juce::ComboBox typeCombo;
    juce::Slider driveSlider;
    juce::Slider toneSlider;
    
    juce::Label onLabel;
    juce::Label typeLabel;
    juce::Label driveLabel;
    juce::Label toneLabel;
    juce::Label titleLabel;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> toneAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortPanel)
};

}