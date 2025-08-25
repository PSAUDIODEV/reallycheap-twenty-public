#include "NoisePanel.h"
#include "../../core/Params.h"

NoisePanel::NoisePanel(juce::AudioProcessorValueTreeState& apvts)
    : valueTreeState(apvts)
{
    // Title
    titleLabel.setText("NOISE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // ON button
    onButton.setButtonText("ON");
    onButton.setClickingTogglesState(true);
    addAndMakeVisible(onButton);
    onAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        valueTreeState, ReallyCheap::ParameterIDs::noiseOn, onButton);
    
    // Type combo
    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.attachToComponent(&typeCombo, false);
    typeCombo.addItemList(ReallyCheap::ParameterHelper::getNoiseTypeChoices(), 1);
    addAndMakeVisible(typeCombo);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, ReallyCheap::ParameterIDs::noiseType, typeCombo);
    
    // Level slider
    levelLabel.setText("Level", juce::dontSendNotification);
    levelLabel.attachToComponent(&levelSlider, false);
    levelSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    levelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    levelSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(levelSlider);
    levelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ReallyCheap::ParameterIDs::noiseLevel, levelSlider);
    
    // Age slider
    ageLabel.setText("Age", juce::dontSendNotification);
    ageLabel.attachToComponent(&ageSlider, false);
    ageSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ageSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(ageSlider);
    ageAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ReallyCheap::ParameterIDs::noiseAge, ageSlider);
    
    // Flutter Gate slider
    flutterGateLabel.setText("Flutter", juce::dontSendNotification);
    flutterGateLabel.attachToComponent(&flutterGateSlider, false);
    flutterGateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    flutterGateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(flutterGateSlider);
    flutterGateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ReallyCheap::ParameterIDs::noiseFlutterGate, flutterGateSlider);
    
    // Width slider
    widthLabel.setText("Width", juce::dontSendNotification);
    widthLabel.attachToComponent(&widthSlider, false);
    widthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(widthSlider);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, ReallyCheap::ParameterIDs::noiseWidth, widthSlider);
    
    // Placement combo
    placementLabel.setText("Place", juce::dontSendNotification);
    placementLabel.attachToComponent(&placementCombo, false);
    placementCombo.addItemList(ReallyCheap::ParameterHelper::getPlacementChoices(), 1);
    addAndMakeVisible(placementCombo);
    placementAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, ReallyCheap::ParameterIDs::noisePlacement, placementCombo);
}

NoisePanel::~NoisePanel()
{
}

void NoisePanel::paint(juce::Graphics& g)
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

void NoisePanel::resized()
{
    auto bounds = getLocalBounds();
    
    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(25));
    
    // ON button
    onButton.setBounds(10, 35, 50, 20);
    
    // Type combo
    typeCombo.setBounds(70, 35, 100, 20);
    
    // Placement combo
    placementCombo.setBounds(180, 35, 60, 20);
    
    // Sliders in a row
    const int sliderTop = 80;
    const int sliderSize = 50;
    const int sliderSpacing = 55;
    
    levelSlider.setBounds(10, sliderTop, sliderSize, sliderSize);
    ageSlider.setBounds(10 + sliderSpacing, sliderTop, sliderSize, sliderSize);
    flutterGateSlider.setBounds(10 + sliderSpacing * 2, sliderTop, sliderSize, sliderSize);
    widthSlider.setBounds(10 + sliderSpacing * 3, sliderTop, sliderSize, sliderSize);
}