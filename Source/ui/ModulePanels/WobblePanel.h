#pragma once

#include <JuceHeader.h>
#include "../../core/Params.h"
#include "../LookAndFeel.h"

namespace ReallyCheap
{

class WobblePanel : public juce::Component
{
public:
    WobblePanel(juce::AudioProcessorValueTreeState& apvts);
    ~WobblePanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    
    // Wobble controls
    juce::ToggleButton onButton;
    juce::Slider depthSlider;
    juce::Slider rateSlider;
    juce::ToggleButton syncButton;
    juce::ToggleButton monoButton;
    juce::Slider flutterSlider;
    juce::Slider driftSlider;
    juce::Slider jitterSlider;
    juce::Slider stereoLinkSlider;
    
    juce::Label onLabel;
    juce::Label depthLabel;
    juce::Label rateLabel;
    juce::Label syncLabel;
    juce::Label monoLabel;
    juce::Label flutterLabel;
    juce::Label driftLabel;
    juce::Label jitterLabel;
    juce::Label stereoLinkLabel;
    juce::Label titleLabel;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flutterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> jitterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoLinkAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WobblePanel)
};

}