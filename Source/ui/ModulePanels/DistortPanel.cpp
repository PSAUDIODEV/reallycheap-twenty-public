#include "DistortPanel.h"

ReallyCheap::DistortPanel::DistortPanel(juce::AudioProcessorValueTreeState& apvts_)
    : apvts(apvts_)
{
    // Title
    titleLabel.setText("DISTORTION", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);
    
    // On/Off toggle
    onButton.setButtonText("ON");
    addAndMakeVisible(onButton);
    onLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(onLabel);
    
    // Type combo
    typeCombo.addItemList(juce::StringArray{"tape", "diode", "fold"}, 1);
    addAndMakeVisible(typeCombo);
    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(typeLabel);
    
    // Drive slider
    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    driveSlider.setRange(0.0, 10.0, 0.1);
    driveSlider.setValue(4.0);
    addAndMakeVisible(driveSlider);
    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);
    
    // Tone slider
    toneSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    toneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    toneSlider.setRange(-1.0, 1.0, 0.01);
    toneSlider.setValue(0.0);
    addAndMakeVisible(toneSlider);
    toneLabel.setText("Tone", juce::dontSendNotification);
    toneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(toneLabel);
    
    // Create attachments
    onAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "distortOn", onButton);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "distortType", typeCombo);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "distortDrive", driveSlider);
    toneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "distortTone", toneSlider);
}

ReallyCheap::DistortPanel::~DistortPanel()
{
    // Empty destructor
}

void ReallyCheap::DistortPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Panel background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    
    // Panel border
    g.setColour(juce::Colour(0xff505050));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
}

void ReallyCheap::DistortPanel::resized()
{
    auto area = getLocalBounds();
    area.reduce(10, 10);
    
    // Title at top
    titleLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(10);
    
    // First row: ON button and Type combo
    auto row1 = area.removeFromTop(60);
    auto buttonWidth = row1.getWidth() / 2 - 5;
    
    auto onArea = row1.removeFromLeft(buttonWidth);
    onButton.setBounds(onArea.removeFromTop(30));
    onLabel.setBounds(onArea);
    
    row1.removeFromLeft(10);
    
    auto typeArea = row1.removeFromLeft(buttonWidth);
    typeCombo.setBounds(typeArea.removeFromTop(30));
    typeLabel.setBounds(typeArea);
    
    area.removeFromTop(10);
    
    // Second row: Drive and Tone sliders
    auto row2 = area.removeFromTop(80);
    auto sliderWidth = row2.getWidth() / 2 - 5;
    
    auto driveArea = row2.removeFromLeft(sliderWidth);
    driveSlider.setBounds(driveArea.removeFromTop(60));
    driveLabel.setBounds(driveArea);
    
    row2.removeFromLeft(10);
    
    auto toneArea = row2.removeFromLeft(sliderWidth);
    toneSlider.setBounds(toneArea.removeFromTop(60));
    toneLabel.setBounds(toneArea);
}