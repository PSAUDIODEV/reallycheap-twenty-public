#include "WobblePanel.h"

ReallyCheap::WobblePanel::WobblePanel(juce::AudioProcessorValueTreeState& apvts_)
    : apvts(apvts_)
{
    // Title
    titleLabel.setText("WOBBLE", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);
    
    // On/Off toggle
    onButton.setButtonText("ON");
    addAndMakeVisible(onButton);
    onLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(onLabel);
    
    // Depth slider
    depthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    depthSlider.setRange(0.0, 1.0, 0.01);
    depthSlider.setValue(0.2);
    addAndMakeVisible(depthSlider);
    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(depthLabel);
    
    // Rate slider
    rateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    rateSlider.setRange(0.1, 12.0, 0.1);
    rateSlider.setValue(1.2);
    addAndMakeVisible(rateSlider);
    rateLabel.setText("Rate", juce::dontSendNotification);
    rateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rateLabel);
    
    // Sync toggle
    syncButton.setButtonText("SYNC");
    addAndMakeVisible(syncButton);
    syncLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(syncLabel);
    
    // Mono toggle
    monoButton.setButtonText("MONO");
    addAndMakeVisible(monoButton);
    monoLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(monoLabel);
    
    // Flutter slider
    flutterSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    flutterSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    flutterSlider.setRange(0.0, 1.0, 0.01);
    flutterSlider.setValue(0.15);
    addAndMakeVisible(flutterSlider);
    flutterLabel.setText("Flutter", juce::dontSendNotification);
    flutterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(flutterLabel);
    
    // Drift slider
    driftSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driftSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    driftSlider.setRange(0.0, 1.0, 0.01);
    driftSlider.setValue(0.25);
    addAndMakeVisible(driftSlider);
    driftLabel.setText("Drift", juce::dontSendNotification);
    driftLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driftLabel);
    
    // Jitter slider
    jitterSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    jitterSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    jitterSlider.setRange(0.0, 1.0, 0.01);
    jitterSlider.setValue(0.1);
    addAndMakeVisible(jitterSlider);
    jitterLabel.setText("Jitter", juce::dontSendNotification);
    jitterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(jitterLabel);
    
    // Stereo Link slider
    stereoLinkSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    stereoLinkSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    stereoLinkSlider.setRange(0.0, 1.0, 0.01);
    stereoLinkSlider.setValue(0.7);
    addAndMakeVisible(stereoLinkSlider);
    stereoLinkLabel.setText("Link", juce::dontSendNotification);
    stereoLinkLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stereoLinkLabel);
    
    // Create attachments
    onAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParameterIDs::wobbleOn, onButton);
    depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::wobbleDepth, depthSlider);
    rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::wobbleRateHz, rateSlider);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParameterIDs::wobbleSync, syncButton);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParameterIDs::wobbleMono, monoButton);
    flutterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::wobbleFlutter, flutterSlider);
    driftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::wobbleDrift, driftSlider);
    jitterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::wobbleJitter, jitterSlider);
    stereoLinkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::wobbleStereoLink, stereoLinkSlider);
}

ReallyCheap::WobblePanel::~WobblePanel()
{
    // Empty destructor
}

void ReallyCheap::WobblePanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Panel background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    
    // Panel border
    g.setColour(juce::Colour(0xff505050));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
}

void ReallyCheap::WobblePanel::resized()
{
    auto area = getLocalBounds();
    area.reduce(10, 10);
    
    // Title at top
    titleLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(10);
    
    // First row: ON, Sync, and Mono buttons
    auto row1 = area.removeFromTop(60);
    auto buttonWidth = row1.getWidth() / 3 - 7;
    
    auto onArea = row1.removeFromLeft(buttonWidth);
    onButton.setBounds(onArea.removeFromTop(30));
    onLabel.setBounds(onArea);
    
    row1.removeFromLeft(10);
    
    auto syncArea = row1.removeFromLeft(buttonWidth);
    syncButton.setBounds(syncArea.removeFromTop(30));
    syncLabel.setBounds(syncArea);
    
    row1.removeFromLeft(10);
    
    auto monoArea = row1.removeFromLeft(buttonWidth);
    monoButton.setBounds(monoArea.removeFromTop(30));
    monoLabel.setBounds(monoArea);
    
    area.removeFromTop(10);
    
    // Second row: Depth and Rate sliders
    auto row2 = area.removeFromTop(80);
    auto sliderWidth = row2.getWidth() / 2 - 5;
    
    auto depthArea = row2.removeFromLeft(sliderWidth);
    depthSlider.setBounds(depthArea.removeFromTop(60));
    depthLabel.setBounds(depthArea);
    
    row2.removeFromLeft(10);
    
    auto rateArea = row2.removeFromLeft(sliderWidth);
    rateSlider.setBounds(rateArea.removeFromTop(60));
    rateLabel.setBounds(rateArea);
    
    area.removeFromTop(10);
    
    // Third row: Flutter and Drift sliders
    auto row3 = area.removeFromTop(80);
    
    auto flutterArea = row3.removeFromLeft(sliderWidth);
    flutterSlider.setBounds(flutterArea.removeFromTop(60));
    flutterLabel.setBounds(flutterArea);
    
    row3.removeFromLeft(10);
    
    auto driftArea = row3.removeFromLeft(sliderWidth);
    driftSlider.setBounds(driftArea.removeFromTop(60));
    driftLabel.setBounds(driftArea);
    
    area.removeFromTop(10);
    
    // Fourth row: Jitter and Stereo Link sliders
    auto row4 = area.removeFromTop(80);
    
    auto jitterArea = row4.removeFromLeft(sliderWidth);
    jitterSlider.setBounds(jitterArea.removeFromTop(60));
    jitterLabel.setBounds(jitterArea);
    
    row4.removeFromLeft(10);
    
    auto stereoLinkArea = row4.removeFromLeft(sliderWidth);
    stereoLinkSlider.setBounds(stereoLinkArea.removeFromTop(60));
    stereoLinkLabel.setBounds(stereoLinkArea);
}