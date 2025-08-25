#pragma once

#include <JuceHeader.h>
#include "../../core/Params.h"

namespace ReallyCheap
{

class MagneticPanel : public juce::Component
{
public:
    MagneticPanel(juce::AudioProcessorValueTreeState& apvts);
    ~MagneticPanel();
    
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    
    // Title
    juce::Label titleLabel;
    
    // Controls
    juce::ToggleButton onButton;
    juce::Label onLabel;
    
    juce::Slider compAmountSlider;
    juce::Label compAmountLabel;
    
    juce::Slider satAmountSlider;
    juce::Label satAmountLabel;
    
    juce::Slider crosstalkSlider;
    juce::Label crosstalkLabel;
    
    juce::Slider headBumpSlider;
    juce::Label headBumpLabel;
    
    juce::Slider wearSlider;
    juce::Label wearLabel;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> satAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crosstalkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> headBumpAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wearAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagneticPanel)
};

}