#pragma once

#include <JuceHeader.h>

namespace ReallyCheap
{

class ReallyCheapLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ReallyCheapLookAndFeel();
    ~ReallyCheapLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH,
                     juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                          const bool isSeparator, const bool isActive,
                          const bool isHighlighted, const bool isTicked,
                          const bool hasSubMenu, const juce::String& text,
                          const juce::String& shortcutKeyText,
                          const juce::Drawable* icon, const juce::Colour* const textColourToUse) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;

    // Figma Design Colors - Exact Match
    static juce::Colour getBackgroundColour() { return juce::Colour(0xff1d7a52); }    // Rich teal background from Figma
    static juce::Colour getPanelColour() { return juce::Colour(0xff165c41); }         // Darker teal for module sections
    static juce::Colour getKnobColour() { return juce::Colour(0xfff4d03f); }          // Golden yellow knobs from Figma
    static juce::Colour getKnobRingColour() { return juce::Colour(0xff0d5734); }      // Dark green knob border from Figma
    static juce::Colour getRedTextColour() { return juce::Colour(0xffdc2626); }       // Red module headers
    static juce::Colour getWhiteTextColour() { return juce::Colour(0xffffffff); }     // White parameter labels
    static juce::Colour getBlackColour() { return juce::Colour(0xff000000); }         // Black elements
    static juce::Colour getPanelBorderColour() { return juce::Colour(0xff000000); }   // Black panel dividers
    static juce::Colour getAccentColour() { return juce::Colour(0xffdc2626); }        // Red accents
    static juce::Colour getDisabledColour() { return juce::Colour(0xff4a5c54); }      // Muted elements

    // Toy-specific styling helpers
    static juce::Font getToyFont(float height) { 
        return juce::Font("Arial Black", height, juce::Font::bold); // Chunky toy font
    }
    static juce::Font getLabelFont(float height) {
        return juce::Font("Comic Sans MS", height, juce::Font::bold); // Playful labels 
    }

private:
    static constexpr float kCornerRadius = 8.0f;     // Rounder for toy aesthetic
    static constexpr float kStrokeThickness = 3.0f;  // Chunkier borders
    
    // Image-based knob assets (Figma design)
    juce::Image largeKnobFilmstrip;
    juce::Image smallKnobFilmstrip;
    int numKnobFrames = 64;
    int largeKnobSize = 168;
    int smallKnobSize = 108;
    
    void loadKnobImages();
    
    // Knob size enumeration for different control types
    enum class KnobSize
    {
        Small,   // 108px - for secondary parameters
        Large    // 168px - for main parameters  
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReallyCheapLookAndFeel)
};

class CustomSlider : public juce::Slider
{
public:
    CustomSlider(const juce::String& parameterID, juce::AudioProcessorValueTreeState& apvts);
    ~CustomSlider() override = default;

    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& key) override;
    void focusGained(juce::Component::FocusChangeType cause) override;
    void focusLost(juce::Component::FocusChangeType cause) override;

    void setTooltipText(const juce::String& tooltip) { tooltipText = tooltip; }
    juce::String getTooltip() override { return tooltipText; }

private:
    juce::String parameterID;
    juce::AudioProcessorValueTreeState& apvts;
    juce::String tooltipText;
    bool keyboardEntryMode = false;
    juce::String keyboardBuffer;

    void resetToDefault();
    void enterKeyboardEntryMode();
    void exitKeyboardEntryMode();
    void processKeyboardEntry();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomSlider)
};

class CustomComboBox : public juce::ComboBox
{
public:
    CustomComboBox(const juce::String& parameterID, juce::AudioProcessorValueTreeState& apvts);
    ~CustomComboBox() override = default;

    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void setTooltipText(const juce::String& tooltip) { tooltipText = tooltip; }
    juce::String getTooltip() override { return tooltipText; }

private:
    juce::String parameterID;
    juce::AudioProcessorValueTreeState& apvts;
    juce::String tooltipText;

    void resetToDefault();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomComboBox)
};

class CustomToggleButton : public juce::ToggleButton
{
public:
    CustomToggleButton(const juce::String& parameterID, juce::AudioProcessorValueTreeState& apvts);
    ~CustomToggleButton() override = default;

    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void setTooltipText(const juce::String& tooltip) { tooltipText = tooltip; }
    juce::String getTooltip() override { return tooltipText; }

private:
    juce::String parameterID;
    juce::AudioProcessorValueTreeState& apvts;
    juce::String tooltipText;

    void resetToDefault();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomToggleButton)
};

}