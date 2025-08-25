#include "DigitalPanel.h"

ReallyCheap::DigitalPanel::DigitalPanel(juce::AudioProcessorValueTreeState& apvts_)
    : apvts(apvts_)
{
    // Title
    titleLabel.setText("DIGITAL", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);
    
    // On/Off toggle
    onButton.setButtonText("ON");
    addAndMakeVisible(onButton);
    onLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(onLabel);
    
    // Bits slider
    bitsSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    bitsSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    bitsSlider.setRange(4.0, 16.0, 1.0);
    bitsSlider.setValue(12.0);
    addAndMakeVisible(bitsSlider);
    bitsLabel.setText("Bits", juce::dontSendNotification);
    bitsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bitsLabel);
    
    // Sample Rate slider
    srSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    srSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    srSlider.setRange(8000.0, 44100.0, 100.0);
    srSlider.setValue(24000.0);
    srSlider.setTextValueSuffix(" Hz");
    addAndMakeVisible(srSlider);
    srLabel.setText("Sample Rate", juce::dontSendNotification);
    srLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(srLabel);
    
    // Jitter slider
    jitterSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    jitterSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    jitterSlider.setRange(0.0, 1.0, 0.01);
    jitterSlider.setValue(0.1);
    addAndMakeVisible(jitterSlider);
    jitterLabel.setText("Jitter", juce::dontSendNotification);
    jitterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(jitterLabel);
    
    // Anti-aliasing toggle
    aaButton.setButtonText("AA");
    addAndMakeVisible(aaButton);
    aaLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(aaLabel);
    
    // Create attachments
    onAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParameterIDs::digitalOn, onButton);
    bitsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::digitalBits, bitsSlider);
    srAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::digitalSR, srSlider);
    jitterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::digitalJitter, jitterSlider);
    aaAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParameterIDs::digitalAA, aaButton);
}

ReallyCheap::DigitalPanel::~DigitalPanel()
{
    // Empty destructor
}

void ReallyCheap::DigitalPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Panel background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    
    // Panel border
    g.setColour(juce::Colour(0xff505050));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
}

void ReallyCheap::DigitalPanel::resized()
{
    auto area = getLocalBounds();
    area.reduce(10, 10);
    
    // Title at top
    titleLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(10);
    
    // First row: ON and AA buttons
    auto row1 = area.removeFromTop(60);
    auto buttonWidth = row1.getWidth() / 2 - 5;
    
    auto onArea = row1.removeFromLeft(buttonWidth);
    onButton.setBounds(onArea.removeFromTop(30));
    onLabel.setBounds(onArea);
    
    row1.removeFromLeft(10);
    
    auto aaArea = row1.removeFromLeft(buttonWidth);
    aaButton.setBounds(aaArea.removeFromTop(30));
    aaLabel.setBounds(aaArea);
    
    area.removeFromTop(10);
    
    // Second row: Bits and Sample Rate sliders
    auto row2 = area.removeFromTop(80);
    auto sliderWidth = row2.getWidth() / 2 - 5;
    
    auto bitsArea = row2.removeFromLeft(sliderWidth);
    bitsSlider.setBounds(bitsArea.removeFromTop(60));
    bitsLabel.setBounds(bitsArea);
    
    row2.removeFromLeft(10);
    
    auto srArea = row2.removeFromLeft(sliderWidth);
    srSlider.setBounds(srArea.removeFromTop(60));
    srLabel.setBounds(srArea);
    
    area.removeFromTop(10);
    
    // Third row: Jitter slider (centered)
    auto row3 = area.removeFromTop(80);
    auto jitterArea = row3.withSizeKeepingCentre(sliderWidth, 80);
    jitterSlider.setBounds(jitterArea.removeFromTop(60));
    jitterLabel.setBounds(jitterArea);
}