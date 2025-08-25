#include "LookAndFeel.h"
#include "../core/Params.h"

namespace ReallyCheap
{

ReallyCheapLookAndFeel::ReallyCheapLookAndFeel()
{
    // Load knob images
    loadKnobImages();
    
    // Fisher Price / Dollar Tree styling
    setColour(juce::ResizableWindow::backgroundColourId, getBackgroundColour());
    
    // Chunky golden knobs
    setColour(juce::Slider::thumbColourId, getKnobColour());
    setColour(juce::Slider::trackColourId, getKnobRingColour());
    setColour(juce::Slider::rotarySliderFillColourId, getKnobColour());
    setColour(juce::Slider::rotarySliderOutlineColourId, getKnobRingColour());
    setColour(juce::Slider::textBoxTextColourId, getWhiteTextColour());
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    
    // Red accent buttons
    setColour(juce::TextButton::buttonColourId, getAccentColour());
    setColour(juce::TextButton::buttonOnColourId, getAccentColour().brighter());
    setColour(juce::TextButton::textColourOffId, getWhiteTextColour());
    setColour(juce::TextButton::textColourOnId, getWhiteTextColour());
    
    // Combo boxes in panel color
    setColour(juce::ComboBox::backgroundColourId, getPanelColour());
    setColour(juce::ComboBox::textColourId, getWhiteTextColour());
    setColour(juce::ComboBox::outlineColourId, getKnobRingColour());
    setColour(juce::ComboBox::buttonColourId, getAccentColour());
    setColour(juce::ComboBox::arrowColourId, getWhiteTextColour());
    
    // Popup menus
    setColour(juce::PopupMenu::backgroundColourId, getPanelColour());
    setColour(juce::PopupMenu::textColourId, getWhiteTextColour());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, getKnobColour());
    setColour(juce::PopupMenu::highlightedTextColourId, getBlackColour());
}

void ReallyCheapLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                            juce::Slider& slider)
{
    // Check if this is one of our transparent global controls
    // These should not draw anything since they overlay the background image
    if (slider.findColour(juce::Slider::rotarySliderFillColourId) == juce::Colours::transparentBlack)
    {
        // This is a transparent overlay control - don't draw anything
        return;
    }
    
    auto bounds = juce::Rectangle<int>(x, y, width, height);
    
    // Determine knob size based on bounds - larger bounds get large knobs
    bool useLargeKnob = width > 80 || height > 80;
    
    if ((useLargeKnob && largeKnobFilmstrip.isValid()) || (!useLargeKnob && smallKnobFilmstrip.isValid()))
    {
        // Select appropriate filmstrip and size
        auto& filmstrip = useLargeKnob ? largeKnobFilmstrip : smallKnobFilmstrip;
        auto knobSize = useLargeKnob ? largeKnobSize : smallKnobSize;
        
        // Calculate which frame to show based on slider position
        int frameIndex = static_cast<int>(sliderPos * (numKnobFrames - 1));
        frameIndex = juce::jlimit(0, numKnobFrames - 1, frameIndex);
        
        // Extract the specific frame from the filmstrip
        juce::Rectangle<int> sourceRect(0, frameIndex * knobSize, knobSize, knobSize);
        juce::Image frameImage = filmstrip.getClippedImage(sourceRect);
        
        // Scale down to fit bounds while maintaining aspect ratio
        auto destSize = juce::jmin(width - 4, height - 4);
        auto destRect = bounds.withSizeKeepingCentre(destSize, destSize);
        
        // Draw the frame image
        g.drawImageWithin(frameImage, destRect.getX(), destRect.getY(), destRect.getWidth(), destRect.getHeight(),
                         juce::RectanglePlacement::centred, false);
    }
    else
    {
        // Fallback to drawn knob using Figma colors
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(3.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        
        // Draw knob body in Figma golden color
        g.setColour(getKnobColour());
        g.fillEllipse(rx, ry, rw, rw);
        
        // Draw dark green border
        g.setColour(getKnobRingColour());
        g.drawEllipse(rx, ry, rw, rw, 3.0f);
        
        // Draw position indicator line
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineLength = radius * 0.7f;
        auto lineX = centreX + std::cos(angle - juce::MathConstants<float>::halfPi) * lineLength;
        auto lineY = centreY + std::sin(angle - juce::MathConstants<float>::halfPi) * lineLength;
        
        g.setColour(getKnobRingColour());
        g.drawLine(centreX, centreY, lineX, lineY, 3.0f);
    }
}

void ReallyCheapLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                const juce::Colour& backgroundColour,
                                                bool shouldDrawButtonAsHighlighted,
                                                bool shouldDrawButtonAsDown)
{
    // Check if this is a transparent overlay button
    if (button.findColour(juce::TextButton::buttonColourId) == juce::Colours::transparentBlack)
    {
        // Check if this is a preset button that needs hover glow effect
        bool isPresetButton = button.getProperties().contains("isPresetButton");
        
        if (isPresetButton && shouldDrawButtonAsHighlighted)
        {
            // Draw a subtle rectangular glow for preset buttons (no corner radius)
            auto bounds = button.getLocalBounds().toFloat();
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.fillRect(bounds);
            
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.drawRect(bounds, 1.0f);
        }
        
        return;
    }
    
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    auto baseColour = backgroundColour;
    
    // Fisher Price button styling - chunky and colorful
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.3f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.2f);
    
    // Draw main button body with toy-like styling
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, kCornerRadius);
    
    // Add 3D highlight effect (top-left highlight)
    g.setColour(baseColour.brighter(0.4f));
    auto highlightBounds = bounds.reduced(4.0f);
    highlightBounds = highlightBounds.withHeight(highlightBounds.getHeight() * 0.3f);
    g.fillRoundedRectangle(highlightBounds, kCornerRadius * 0.5f);
    
    // Chunky black border for that toy contrast
    g.setColour(getPanelBorderColour());
    g.drawRoundedRectangle(bounds, kCornerRadius, kStrokeThickness);
}

void ReallyCheapLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                        int buttonX, int buttonY, int buttonW, int buttonH,
                                        juce::ComboBox& box)
{
    // Check if this is a transparent overlay combo box
    if (box.findColour(juce::ComboBox::backgroundColourId) == juce::Colours::transparentBlack)
    {
        // This is a transparent overlay combo box - don't draw anything
        return;
    }
    
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(0.5f);
    
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, kCornerRadius);
    
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, kCornerRadius, kStrokeThickness);

    juce::Rectangle<int> arrowZone(width - 30, 0, 20, height);
    juce::Path path;
    path.startNewSubPath(arrowZone.getX() + 3.0f, arrowZone.getCentreY() - 2.0f);
    path.lineTo(static_cast<float>(arrowZone.getCentreX()), arrowZone.getCentreY() + 3.0f);
    path.lineTo(arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(box.isEnabled() ? 0.9f : 0.2f));
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void ReallyCheapLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(findColour(juce::PopupMenu::backgroundColourId));
    
    g.setColour(getKnobRingColour());
    g.drawRect(0, 0, width, height, 1);
}

void ReallyCheapLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                             const bool isSeparator, const bool isActive,
                                             const bool isHighlighted, const bool isTicked,
                                             const bool hasSubMenu, const juce::String& text,
                                             const juce::String& shortcutKeyText,
                                             const juce::Drawable* icon, const juce::Colour* const textColourToUse)
{
    if (isSeparator)
    {
        auto r = area.reduced(5, 0);
        r.removeFromTop(juce::roundToInt((r.getHeight() * 0.5f) - 0.5f));

        g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.3f));
        g.fillRect(r.removeFromTop(1));
    }
    else
    {
        auto textColour = textColourToUse == nullptr ? findColour(juce::PopupMenu::textColourId) : *textColourToUse;

        auto r = area.reduced(1);

        if (isHighlighted && isActive)
        {
            g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
            g.fillRoundedRectangle(r.toFloat(), 2.0f);

            g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
        }
        else
        {
            g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
        }

        r.reduce(juce::jmin(5, area.getWidth() / 20), 0);

        auto font = getPopupMenuFont();

        auto maxFontHeight = (float) r.getHeight() / 1.3f;

        if (font.getHeight() > maxFontHeight)
            font.setHeight(maxFontHeight);

        g.setFont(font);

        auto iconArea = r.removeFromLeft(juce::roundToInt(maxFontHeight)).toFloat();

        if (icon != nullptr)
        {
            icon->drawWithin(g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
            r.removeFromLeft(juce::roundToInt(maxFontHeight * 0.5f));
        }
        else if (isTicked)
        {
            auto tick = getTickShape(1.0f);
            g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
        }

        if (hasSubMenu)
        {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();

            auto x = static_cast<float>(r.removeFromRight((int) arrowH).getX());
            auto halfH = static_cast<float>(r.getCentreY());

            juce::Path path;
            path.startNewSubPath(x, halfH - arrowH * 0.5f);
            path.lineTo(x + arrowH * 0.6f, halfH);
            path.lineTo(x, halfH + arrowH * 0.5f);

            g.strokePath(path, juce::PathStrokeType(2.0f));
        }

        r.removeFromRight(3);
        g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            auto f2 = font;
            f2.setHeight(f2.getHeight() * 0.75f);
            f2.setHorizontalScale(0.95f);
            g.setFont(f2);

            g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
        }
    }
}

juce::Font ReallyCheapLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return getToyFont(juce::jmin(15.0f, buttonHeight * 0.6f));
}

juce::Font ReallyCheapLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return getToyFont(12.0f);
}

juce::Font ReallyCheapLookAndFeel::getPopupMenuFont()
{
    return getToyFont(12.0f);
}

void ReallyCheapLookAndFeel::loadKnobImages()
{
    DBG("Loading Figma knob filmstrips...");
    
    // Base paths to try
    juce::Array<juce::File> basePaths;
    basePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets").getChildFile("knobs"));
    basePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("assets").getChildFile("knobs"));
    basePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets\\knobs"));
    
    for (auto& basePath : basePaths)
    {
        DBG("Trying asset path: " << basePath.getFullPathName());
        
        // Try to load large knob filmstrip
        auto largeKnobFile = basePath.getChildFile("large_knob_filmstrip.png");
        if (largeKnobFile.existsAsFile() && !largeKnobFilmstrip.isValid())
        {
            largeKnobFilmstrip = juce::ImageFileFormat::loadFrom(largeKnobFile);
            if (largeKnobFilmstrip.isValid())
            {
                DBG("SUCCESS: Loaded large knob filmstrip: " << largeKnobSize << "x" << largeKnobSize);
            }
        }
        
        // Try to load small knob filmstrip
        auto smallKnobFile = basePath.getChildFile("small_knob_filmstrip.png");
        if (smallKnobFile.existsAsFile() && !smallKnobFilmstrip.isValid())
        {
            smallKnobFilmstrip = juce::ImageFileFormat::loadFrom(smallKnobFile);
            if (smallKnobFilmstrip.isValid())
            {
                DBG("SUCCESS: Loaded small knob filmstrip: " << smallKnobSize << "x" << smallKnobSize);
            }
        }
        
        // If we have both, we're done
        if (largeKnobFilmstrip.isValid() && smallKnobFilmstrip.isValid())
        {
            DBG("Both knob filmstrips loaded successfully!");
            return;
        }
    }
    
    if (!largeKnobFilmstrip.isValid())
        DBG("WARNING: Could not find large knob filmstrip");
    if (!smallKnobFilmstrip.isValid())
        DBG("WARNING: Could not find small knob filmstrip");
    
    DBG("Using fallback drawn knobs where filmstrips are missing");
}

// CustomSlider Implementation
CustomSlider::CustomSlider(const juce::String& parameterID_, juce::AudioProcessorValueTreeState& apvts_)
    : parameterID(parameterID_), apvts(apvts_)
{
    setWantsKeyboardFocus(true);
}

void CustomSlider::mouseDoubleClick(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    resetToDefault();
}

void CustomSlider::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        grabKeyboardFocus();
        enterKeyboardEntryMode();
    }
    else
    {
        juce::Slider::mouseDown(event);
    }
}

bool CustomSlider::keyPressed(const juce::KeyPress& key)
{
    if (keyboardEntryMode)
    {
        if (key == juce::KeyPress::returnKey || key == juce::KeyPress::escapeKey)
        {
            if (key == juce::KeyPress::returnKey)
                processKeyboardEntry();
            exitKeyboardEntryMode();
            return true;
        }
        else if (key == juce::KeyPress::backspaceKey)
        {
            if (!keyboardBuffer.isEmpty())
                keyboardBuffer = keyboardBuffer.dropLastCharacters(1);
            repaint();
            return true;
        }
        else if (key.getTextCharacter() >= '0' && key.getTextCharacter() <= '9' ||
                 key.getTextCharacter() == '.' || key.getTextCharacter() == '-')
        {
            keyboardBuffer += key.getTextCharacter();
            repaint();
            return true;
        }
    }
    
    return juce::Slider::keyPressed(key);
}

void CustomSlider::focusGained(juce::Component::FocusChangeType cause)
{
    juce::Slider::focusGained(cause);
    repaint();
}

void CustomSlider::focusLost(juce::Component::FocusChangeType cause)
{
    exitKeyboardEntryMode();
    juce::Slider::focusLost(cause);
    repaint();
}

void CustomSlider::resetToDefault()
{
    if (auto* param = apvts.getParameter(parameterID))
    {
        float defaultValue = param->getDefaultValue();
        auto normalizedValue = param->convertFrom0to1(defaultValue);
        setValue(normalizedValue, juce::sendNotificationSync);
    }
}

void CustomSlider::enterKeyboardEntryMode()
{
    keyboardEntryMode = true;
    keyboardBuffer.clear();
    repaint();
}

void CustomSlider::exitKeyboardEntryMode()
{
    keyboardEntryMode = false;
    keyboardBuffer.clear();
    repaint();
}

void CustomSlider::processKeyboardEntry()
{
    if (keyboardBuffer.isNotEmpty())
    {
        double newValue = keyboardBuffer.getDoubleValue();
        double clampedValue = juce::jlimit(getMinimum(), getMaximum(), newValue);
        setValue(clampedValue, juce::sendNotificationSync);
    }
}

// CustomComboBox Implementation
CustomComboBox::CustomComboBox(const juce::String& parameterID_, juce::AudioProcessorValueTreeState& apvts_)
    : parameterID(parameterID_), apvts(apvts_)
{
}

void CustomComboBox::mouseDoubleClick(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    resetToDefault();
}

void CustomComboBox::resetToDefault()
{
    if (auto* param = apvts.getParameter(parameterID))
    {
        float defaultValue = param->getDefaultValue();
        setSelectedItemIndex(static_cast<int>(defaultValue), juce::sendNotificationSync);
    }
}

// CustomToggleButton Implementation
CustomToggleButton::CustomToggleButton(const juce::String& parameterID_, juce::AudioProcessorValueTreeState& apvts_)
    : parameterID(parameterID_), apvts(apvts_)
{
}

void CustomToggleButton::mouseDoubleClick(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    resetToDefault();
}

void CustomToggleButton::resetToDefault()
{
    if (auto* param = apvts.getParameter(parameterID))
    {
        bool defaultValue = param->getDefaultValue() >= 0.5f;
        setToggleState(defaultValue, juce::sendNotificationSync);
    }
}

}