#include "SpacePanel.h"
#include "../../core/Params.h"

namespace ReallyCheap
{

SpacePanel::SpacePanel(juce::AudioProcessorValueTreeState& apvts)
    : valueTreeState(apvts)
{
    // Title
    titleLabel.setText("SPACE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // ON button
    onButton.setButtonText("ON");
    onButton.setClickingTogglesState(true);
    addAndMakeVisible(onButton);
    onAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        valueTreeState, ParameterIDs::spaceOn, onButton);
    
    // Mix slider
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.attachToComponent(&mixSlider, false);
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    addAndMakeVisible(mixSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ParameterIDs::spaceMix, mixSlider);
    
    // Time slider
    timeLabel.setText("Time", juce::dontSendNotification);
    timeLabel.attachToComponent(&timeSlider, false);
    timeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    timeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    timeSlider.setTextValueSuffix(" s");
    addAndMakeVisible(timeSlider);
    timeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ParameterIDs::spaceTime, timeSlider);
    
    // Tone slider
    toneLabel.setText("Tone", juce::dontSendNotification);
    toneLabel.attachToComponent(&toneSlider, false);
    toneSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    toneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    addAndMakeVisible(toneSlider);
    toneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ParameterIDs::spaceTone, toneSlider);
    
    // Pre-delay slider
    preDelayLabel.setText("Pre-Dly", juce::dontSendNotification);
    preDelayLabel.attachToComponent(&preDelaySlider, false);
    preDelaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    preDelaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    preDelaySlider.setTextValueSuffix(" ms");
    addAndMakeVisible(preDelaySlider);
    preDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ParameterIDs::spacePreDelayMs, preDelaySlider);
    
    // Cheapo slider
    cheapoLabel.setText("Cheapo", juce::dontSendNotification);
    cheapoLabel.attachToComponent(&cheapoSlider, false);
    cheapoSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    cheapoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    addAndMakeVisible(cheapoSlider);
    cheapoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ParameterIDs::spaceCheapo, cheapoSlider);
}

SpacePanel::~SpacePanel()
{
}

void SpacePanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Draw border
    g.setColour(juce::Colour(0xff444444));
    g.drawRect(getLocalBounds(), 1);
    
    // Draw module background
    g.setColour(juce::Colour(0xff333333));
    g.fillRoundedRectangle(5.0f, 30.0f, static_cast<float>(getWidth() - 10), 
                           static_cast<float>(getHeight() - 35), 5.0f);
}

void SpacePanel::resized()
{
    auto bounds = getLocalBounds();
    
    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(25));
    
    // ON button
    onButton.setBounds(10, 35, 50, 20);
    
    // First row of sliders (Mix, Time, Tone)
    const int firstRowTop = 80;
    const int sliderSize = 45;
    const int sliderSpacing = 50;
    
    mixSlider.setBounds(5, firstRowTop, sliderSize, sliderSize);
    timeSlider.setBounds(5 + sliderSpacing, firstRowTop, sliderSize, sliderSize);
    toneSlider.setBounds(5 + sliderSpacing * 2, firstRowTop, sliderSize, sliderSize);
    
    // Second row of sliders (Pre-delay, Cheapo)
    const int secondRowTop = 160;
    
    preDelaySlider.setBounds(5, secondRowTop, sliderSize, sliderSize);
    cheapoSlider.setBounds(5 + sliderSpacing, secondRowTop, sliderSize, sliderSize);
}

}