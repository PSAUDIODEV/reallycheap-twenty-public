#pragma once

#include <JuceHeader.h>

namespace ReallyCheap
{

class SpacePanel : public juce::Component
{
public:
    SpacePanel(juce::AudioProcessorValueTreeState& apvts);
    ~SpacePanel() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    // Controls
    juce::ToggleButton onButton;
    juce::Slider mixSlider;
    juce::Slider timeSlider;
    juce::Slider toneSlider;
    juce::Slider preDelaySlider;
    juce::Slider cheapoSlider;
    
    // Labels
    juce::Label titleLabel;
    juce::Label mixLabel;
    juce::Label timeLabel;
    juce::Label toneLabel;
    juce::Label preDelayLabel;
    juce::Label cheapoLabel;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> toneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preDelayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cheapoAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpacePanel)
};

}