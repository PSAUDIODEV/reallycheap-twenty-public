#pragma once

#include <JuceHeader.h>
#include "../../core/Params.h"

namespace ReallyCheap
{

class DigitalPanel : public juce::Component
{
public:
    DigitalPanel(juce::AudioProcessorValueTreeState& apvts);
    ~DigitalPanel();
    
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    
    // Title
    juce::Label titleLabel;
    
    // Controls
    juce::ToggleButton onButton;
    juce::Label onLabel;
    
    juce::Slider bitsSlider;
    juce::Label bitsLabel;
    
    juce::Slider srSlider;
    juce::Label srLabel;
    
    juce::Slider jitterSlider;
    juce::Label jitterLabel;
    
    juce::ToggleButton aaButton;
    juce::Label aaLabel;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> srAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> jitterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aaAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigitalPanel)
};

}