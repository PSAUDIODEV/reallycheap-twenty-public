#include "MagneticPanel.h"

ReallyCheap::MagneticPanel::MagneticPanel(juce::AudioProcessorValueTreeState& apvts_)
    : apvts(apvts_)
{
    // Title
    titleLabel.setText("MAGNETIC", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);
    
    // On/Off toggle
    onButton.setButtonText("ON");
    addAndMakeVisible(onButton);
    onLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(onLabel);
    
    // Comp Amount slider
    compAmountSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    compAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    compAmountSlider.setRange(0.0, 1.0, 0.01);
    compAmountSlider.setValue(0.3);
    addAndMakeVisible(compAmountSlider);
    compAmountLabel.setText("Comp", juce::dontSendNotification);
    compAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(compAmountLabel);
    
    // Sat Amount slider
    satAmountSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    satAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    satAmountSlider.setRange(0.0, 1.0, 0.01);
    satAmountSlider.setValue(0.2);
    addAndMakeVisible(satAmountSlider);
    satAmountLabel.setText("Sat", juce::dontSendNotification);
    satAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(satAmountLabel);
    
    // Crosstalk slider
    crosstalkSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    crosstalkSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    crosstalkSlider.setRange(0.0, 1.0, 0.01);
    crosstalkSlider.setValue(0.15);
    addAndMakeVisible(crosstalkSlider);
    crosstalkLabel.setText("X-Talk", juce::dontSendNotification);
    crosstalkLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(crosstalkLabel);
    
    // Head Bump slider
    headBumpSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    headBumpSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    headBumpSlider.setRange(0.0, 1.0, 0.01);
    headBumpSlider.setValue(0.25);
    addAndMakeVisible(headBumpSlider);
    headBumpLabel.setText("Head", juce::dontSendNotification);
    headBumpLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(headBumpLabel);
    
    // Wear slider
    wearSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wearSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    wearSlider.setRange(0.0, 1.0, 0.01);
    wearSlider.setValue(0.1);
    addAndMakeVisible(wearSlider);
    wearLabel.setText("Wear", juce::dontSendNotification);
    wearLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wearLabel);
    
    // Create attachments
    onAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParameterIDs::magOn, onButton);
    compAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::magComp, compAmountSlider);
    satAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::magSat, satAmountSlider);
    crosstalkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::magCrosstalk, crosstalkSlider);
    headBumpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::magHeadBumpHz, headBumpSlider);
    wearAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::magWear, wearSlider);
}

ReallyCheap::MagneticPanel::~MagneticPanel()
{
    // Empty destructor
}

void ReallyCheap::MagneticPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Panel background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    
    // Panel border
    g.setColour(juce::Colour(0xff505050));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
}

void ReallyCheap::MagneticPanel::resized()
{
    auto area = getLocalBounds();
    area.reduce(10, 10);
    
    // Title at top
    titleLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(10);
    
    // First row: ON button (centered)
    auto row1 = area.removeFromTop(60);
    auto buttonWidth = row1.getWidth() / 3; // Center it
    
    auto onArea = row1.withSizeKeepingCentre(buttonWidth, 60);
    onButton.setBounds(onArea.removeFromTop(30));
    onLabel.setBounds(onArea);
    
    area.removeFromTop(10);
    
    // Second row: Comp and Sat sliders
    auto row2 = area.removeFromTop(80);
    auto sliderWidth = row2.getWidth() / 2 - 5;
    
    auto compArea = row2.removeFromLeft(sliderWidth);
    compAmountSlider.setBounds(compArea.removeFromTop(60));
    compAmountLabel.setBounds(compArea);
    
    row2.removeFromLeft(10);
    
    auto satArea = row2.removeFromLeft(sliderWidth);
    satAmountSlider.setBounds(satArea.removeFromTop(60));
    satAmountLabel.setBounds(satArea);
    
    area.removeFromTop(10);
    
    // Third row: Crosstalk and Head Bump sliders
    auto row3 = area.removeFromTop(80);
    
    auto crosstalkArea = row3.removeFromLeft(sliderWidth);
    crosstalkSlider.setBounds(crosstalkArea.removeFromTop(60));
    crosstalkLabel.setBounds(crosstalkArea);
    
    row3.removeFromLeft(10);
    
    auto headArea = row3.removeFromLeft(sliderWidth);
    headBumpSlider.setBounds(headArea.removeFromTop(60));
    headBumpLabel.setBounds(headArea);
    
    area.removeFromTop(10);
    
    // Fourth row: Wear slider (centered)
    auto row4 = area.removeFromTop(80);
    auto wearArea = row4.withSizeKeepingCentre(sliderWidth, 80);
    wearSlider.setBounds(wearArea.removeFromTop(60));
    wearLabel.setBounds(wearArea);
}