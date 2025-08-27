#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

ReallyCheapTwentyAudioProcessorEditor::ReallyCheapTwentyAudioProcessorEditor(ReallyCheapTwentyAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Temporarily disable custom LookAndFeel to test SVG loading
    // lookAndFeel = std::make_unique<ReallyCheap::ReallyCheapLookAndFeel>();
    // setLookAndFeel(lookAndFeel.get());
    
    // Temporarily hide module panels to see full SVG background
    distortPanel = std::make_unique<ReallyCheap::DistortPanel>(audioProcessor.getValueTreeState());
    // addAndMakeVisible(*distortPanel);
    
    wobblePanel = std::make_unique<ReallyCheap::WobblePanel>(audioProcessor.getValueTreeState());
    // addAndMakeVisible(*wobblePanel);
    
    digitalPanel = std::make_unique<ReallyCheap::DigitalPanel>(audioProcessor.getValueTreeState());
    // addAndMakeVisible(*digitalPanel);
    
    magneticPanel = std::make_unique<ReallyCheap::MagneticPanel>(audioProcessor.getValueTreeState());
    // addAndMakeVisible(*magneticPanel);
    
    noisePanel = std::make_unique<NoisePanel>(audioProcessor.getValueTreeState());
    // addAndMakeVisible(*noisePanel);
    
    spacePanel = std::make_unique<ReallyCheap::SpacePanel>(audioProcessor.getValueTreeState());
    // addAndMakeVisible(*spacePanel);
    
    // Setup all module controls
    setupMainKnobs();
    setupWobbleKnobs();        // Bend module (formerly Wobble)
    setupWobbleSwitches();     // Bend module switches
    setupDigitalKnobs();       // Bitcrush module (formerly Digital)
    setupDigitalSwitches();    // Bitcrush module switches
    setupDistortKnobs();       // Crunch module (formerly Distortion)
    setupDistortControls();    // Crunch module controls
    setupMagneticKnobs();      // Tape module (formerly Magnetic)
    setupMagneticControls();   // Tape module controls
    setupNoiseKnobs();         // Atmosphere module (formerly Noise)
    setupNoiseControls();      // Atmosphere module controls
    setupSpaceKnobs();         // Verb module (formerly Space)
    setupSpaceControls();      // Verb module controls
    setupPresetControls();
    // DISABLE ALL ASSET LOADING - Mac debugging (minimal plugin)
    DBG("Mac debug mode: all asset loading disabled for compatibility testing");
    
    // loadBackgroundSVG();           // DISABLED 
    // loadKnobSVGs();                // DISABLED
    // loadWobbleKnobSVGs();          // DISABLED
    // loadWobbleSwitchSVGs();        // DISABLED
    // loadDigitalKnobSVGs();         // DISABLED
    // loadDigitalSwitchSVGs();       // DISABLED
    // loadDistortKnobSVGs();         // DISABLED
    // loadDistortControlSVGs();      // DISABLED
    // loadMagneticKnobSVGs();        // DISABLED
    // loadMagneticControlSVGs();     // DISABLED
    // loadNoiseKnobSVGs();           // DISABLED
    // loadNoiseControlSVGs();        // DISABLED
    // loadSpaceKnobSVGs();           // DISABLED
    // loadSpaceControlSVGs();        // DISABLED
    // loadTitleCardImage();          // DISABLED
    // loadPresetSelectorSVGs();      // DISABLED
    // loadCustomFont();              // DISABLED
    
    // Update fonts after custom font is loaded - DISABLED for Mac debugging
    // noiseTypeLabel.setFont(customFont.withHeight(14.0f));  // DISABLED
    
    // Set up resize handle
    resizer = std::make_unique<juce::ResizableCornerComponent>(this, &resizeConstrainer);
    addAndMakeVisible(resizer.get());
    
    // Set size constraints (allow 50% to 150% of base size)
    int baseWidth = 1074;  // 50% of Figma size
    int baseHeight = 598;
    resizeConstrainer.setMinimumSize(baseWidth * 0.5f, baseHeight * 0.5f);     // 25% of Figma
    resizeConstrainer.setMaximumSize(baseWidth * 3.0f, baseHeight * 3.0f);     // 150% of Figma  
    resizeConstrainer.setFixedAspectRatio(static_cast<double>(baseWidth) / baseHeight);
    
    // Set initial size (50% of Figma size)
    setSize(baseWidth, baseHeight);
}

ReallyCheapTwentyAudioProcessorEditor::~ReallyCheapTwentyAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void ReallyCheapTwentyAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Draw the Figma background SVG
    if (backgroundSVG != nullptr)
    {
        backgroundSVG->drawWithin(g, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit, 1.0f);
    }
    else
    {
        // Fallback to solid color if SVG not loaded
        g.setColour(juce::Colours::darkgreen);
        g.fillAll();
    }
    
    // Draw custom SVG knobs with rotation based on slider values
    if (inputGainKnobSVG != nullptr)
    {
        auto bounds = inGainSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        // Calculate rotation angle based on slider value (-150° to +150°)
        auto normalizedValue = (inGainSlider.getValue() - inGainSlider.getMinimum()) / 
                              (inGainSlider.getMaximum() - inGainSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f); // -150° to +150°
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        inputGainKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    if (outputGainKnobSVG != nullptr)
    {
        auto bounds = outGainSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (outGainSlider.getValue() - outGainSlider.getMinimum()) / 
                              (outGainSlider.getMaximum() - outGainSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        outputGainKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    if (mixKnobSVG != nullptr)
    {
        auto bounds = mixSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (mixSlider.getValue() - mixSlider.getMinimum()) / 
                              (mixSlider.getMaximum() - mixSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        mixKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    if (macroKnobSVG != nullptr)
    {
        auto bounds = macroSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (macroSlider.getValue() - macroSlider.getMinimum()) / 
                              (macroSlider.getMaximum() - macroSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        macroKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw all module controls
    drawWobbleKnobs(g);        // Bend module (formerly Wobble)
    drawWobbleSwitches(g);     // Bend module switches
    drawDigitalKnobs(g);       // Bitcrush module (formerly Digital)
    drawDigitalSwitches(g);    // Bitcrush module switches
    drawDistortKnobs(g);       // Crunch module (formerly Distortion)
    drawDistortControls(g);    // Crunch module controls
    drawMagneticKnobs(g);      // Tape module (formerly Magnetic)
    drawMagneticControls(g);   // Tape module controls
    drawNoiseKnobs(g);         // Atmosphere module (formerly Noise)
    drawNoiseControls(g);      // Atmosphere module controls
    drawSpaceKnobs(g);         // Verb module (formerly Space)
    drawSpaceControls(g);      // Verb module controls
    drawPresetSelector(g);
    
    // Draw title card PNG on top
    if (titleCardImage.isValid())
    {
        DBG("Drawing title card cleaner PNG");
        
        // Get current scale factor (same as all other UI elements)
        float scale = static_cast<float>(getWidth()) / 1908.0f;
        
        // Get actual image dimensions
        auto imageWidth = titleCardImage.getWidth();
        auto imageHeight = titleCardImage.getHeight();
        
        // Base dimensions and position (medium size for optimal visibility)
        float baseWidth = 625.0f; // Medium base width for optimal visibility
        float baseHeight = 385.0f; // Medium base height (maintaining aspect ratio)
        float baseX = -7.0f; // Base X position (adjusted)
        float baseY = -15.0f; // Base Y position (moved up slightly)
        
        // Apply dynamic scaling
        float scaledWidth = baseWidth * scale;
        float scaledHeight = baseHeight * scale;
        float scaledX = baseX * scale;
        float scaledY = baseY * scale;
        
        DBG("Title card scale: " << scale << ", Final size: " << scaledWidth << "x" << scaledHeight);
        
        juce::Rectangle<float> titleBounds(scaledX, scaledY, scaledWidth, scaledHeight);
        g.drawImageWithin(titleCardImage, titleBounds.getX(), titleBounds.getY(), 
                         titleBounds.getWidth(), titleBounds.getHeight(),
                         juce::RectanglePlacement::centred, false);
    }
    else
    {
        DBG("Title card PNG not loaded");
        
        // Get current scale factor for placeholder
        float scale = static_cast<float>(getWidth()) / 1908.0f;
        
        // Draw a placeholder red rectangle to show where title should be (scaled)
        int placeholderX = static_cast<int>(50 * scale);
        int placeholderY = static_cast<int>(50 * scale);
        int placeholderW = static_cast<int>(300 * scale);
        int placeholderH = static_cast<int>(150 * scale);
        
        g.setColour(juce::Colours::red);
        g.fillRect(placeholderX, placeholderY, placeholderW, placeholderH);
        g.setColour(juce::Colours::white);
        g.drawText("TITLE CARD PNG NOT LOADED", placeholderX, placeholderY, placeholderW, placeholderH, juce::Justification::centred);
    }
}

void ReallyCheapTwentyAudioProcessorEditor::resized()
{
    // Calculate current scale based on window size
    // Base size is 1074x598 (50% of Figma), so scale relative to that
    float baseWidth = 1074.0f;
    float scale = getWidth() / baseWidth;
    currentUIScale = scale;
    
    // Position resize handle in bottom-right corner
    const int resizerSize = 16;
    resizer->setBounds(getWidth() - resizerSize, getHeight() - resizerSize, resizerSize, resizerSize);
    
    // All coordinates below are for the base size (1074x598)
    // Apply current scale to resize everything proportionally
    int knobSize = static_cast<int>(80 * scale); // Base knob size: 80px (was 160 * 0.5)
    
    // Original positions (after adjusting for plugin window offset): 
    // INPUT GAIN: 1168, 85 -> scaled: 584, 42.5
    // OUTPUT GAIN: 1407, 85 -> scaled: 703.5, 42.5  
    // MIX: 1631, 85 -> scaled: 815.5, 42.5
    // REALLYCHEAP MACRO: 1870, 85 -> scaled: 935, 42.5
    
    inGainSlider.setBounds(static_cast<int>(584 * scale), static_cast<int>(42 * scale), knobSize, knobSize);      // INPUT GAIN
    outGainSlider.setBounds(static_cast<int>(704 * scale), static_cast<int>(42 * scale), knobSize, knobSize);     // OUTPUT GAIN
    mixSlider.setBounds(static_cast<int>(816 * scale), static_cast<int>(42 * scale), knobSize, knobSize);         // MIX
    macroSlider.setBounds(static_cast<int>(935 * scale), static_cast<int>(42 * scale), knobSize, knobSize);       // REALLYCHEAP MACRO
    
    // Bend module knobs (formerly Wobble - all 100x100 in Figma, base size 50x50)
    int wobbleKnobSize = static_cast<int>(50 * scale); // Base size: 50px (was 100 * 0.5)
    
    // Figma coordinates adjusted for plugin window offset (-175, 1262), then scaled by 50%
    // Depth: -118 x 1948 -> adjusted: (57, 686) -> scaled: (28.5, 343)
    wobbleDepthSlider.setBounds(static_cast<int>(29 * scale), static_cast<int>(343 * scale), wobbleKnobSize, wobbleKnobSize);
    
    // Rate: 43 x 1948 -> adjusted: (218, 686) -> scaled: (109, 343)  
    wobbleRateSlider.setBounds(static_cast<int>(109 * scale), static_cast<int>(343 * scale), wobbleKnobSize, wobbleKnobSize);
    
    // Flutter: -118 x 2116 -> adjusted: (57, 854) -> scaled: (28.5, 427)
    wobbleFlutterSlider.setBounds(static_cast<int>(29 * scale), static_cast<int>(427 * scale), wobbleKnobSize, wobbleKnobSize);
    
    // Drift: 43 x 2116 -> adjusted: (218, 854) -> scaled: (109, 427)
    wobbleDriftSlider.setBounds(static_cast<int>(109 * scale), static_cast<int>(427 * scale), wobbleKnobSize, wobbleKnobSize);
    
    // Jitter: -118 x 2284 -> adjusted: (57, 1022) -> scaled: (28.5, 511)
    wobbleJitterSlider.setBounds(static_cast<int>(29 * scale), static_cast<int>(511 * scale), wobbleKnobSize, wobbleKnobSize);
    
    // Link: 43 x 2284 -> adjusted: (218, 1022) -> scaled: (109, 511)
    wobbleLinkSlider.setBounds(static_cast<int>(109 * scale), static_cast<int>(511 * scale), wobbleKnobSize, wobbleKnobSize);
    
    // Bend module switches (formerly Wobble - all 72x44 in Figma, scaled to 36x22)
    int switchWidth = static_cast<int>(36 * scale); // Base: 36px (was 72 * 0.5)
    int switchHeight = static_cast<int>(22 * scale); // Base: 22px (was 44 * 0.5)
    
    // Figma coordinates adjusted for plugin window offset (-175, 1262), then scaled by 50%
    // ON: -106 x 1687 -> adjusted: (69, 425) -> scaled: (34.5, 212.5) -> up 2 pixels
    wobbleOnButton.setBounds(static_cast<int>(35 * scale), static_cast<int>(211 * scale), switchWidth, switchHeight);
    
    // SYNC: 57 x 1770 -> adjusted: (232, 508) -> scaled: (116, 254) -> left 1 pixel
    wobbleSyncButton.setBounds(static_cast<int>(116 * scale), static_cast<int>(254 * scale), switchWidth, switchHeight);
    
    // MONO: -106 x 1848 -> adjusted: (69, 586) -> scaled: (34.5, 293) -> up 1 pixel
    wobbleMonoButton.setBounds(static_cast<int>(35 * scale), static_cast<int>(292 * scale), switchWidth, switchHeight);
    
    // Bitcrush module knobs (formerly Digital - all 100x100 in Figma, base size 50x50)
    int digitalKnobSize = static_cast<int>(50 * scale); // Base: 50px (was 100 * 0.5)
    
    // Bits knob: 668 x 1949 -> adjusted: (843, 687) -> scaled: (421.5, 343.5)
    digitalBitsSlider.setBounds(static_cast<int>(422 * scale), static_cast<int>(344 * scale), digitalKnobSize, digitalKnobSize);
    
    // SR knob: 668 x 2116 -> adjusted: (843, 854) -> scaled: (421.5, 427)
    digitalSRSlider.setBounds(static_cast<int>(422 * scale), static_cast<int>(427 * scale), digitalKnobSize, digitalKnobSize);
    
    // Jitter knob: 668 x 2284 -> adjusted: (843, 1022) -> scaled: (421.5, 511)
    digitalJitterSlider.setBounds(static_cast<int>(422 * scale), static_cast<int>(511 * scale), digitalKnobSize, digitalKnobSize);
    
    // Bitcrush module switches (formerly Digital - all 72x44 in Figma, scaled to 36x22)
    int digitalSwitchWidth = static_cast<int>(36 * scale); // Base: 36px (was 72 * 0.5)
    int digitalSwitchHeight = static_cast<int>(22 * scale); // Base: 22px (was 44 * 0.5)
    
    // ON switch: 596 x 1694 -> adjusted: (771, 432) -> scaled: (385.5, 216)
    digitalOnButton.setBounds(static_cast<int>(386 * scale), static_cast<int>(216 * scale), digitalSwitchWidth, digitalSwitchHeight);
    
    // AA switch: 768 x 1787 -> adjusted: (943, 525) -> scaled: (471.5, 262.5) -> up 2 pixels
    digitalAAButton.setBounds(static_cast<int>(472 * scale), static_cast<int>(261 * scale), digitalSwitchWidth, digitalSwitchHeight);
    
    // Preset selector - using base dimensions from Figma design
    int presetWidth = static_cast<int>(202 * scale); // Base: 202px (was 403 * 0.5)
    int presetHeight = static_cast<int>(38 * scale); // Base: 38px (was 76 * 0.5)
    
    // Preset selector: fine-tuned position
    presetComboBox.setBounds(static_cast<int>(331 * scale), static_cast<int>(86 * scale), presetWidth, presetHeight);
    
    // Position Save and Load buttons over SVG buttons (invisible but functional)
    // Load button: using base dimensions
    int loadButtonWidth = static_cast<int>(48 * scale); // Base: 48px (was 96 * 0.5)
    int loadButtonHeight = static_cast<int>(21 * scale); // Base: 21px (was 42 * 0.5)
    loadButton.setBounds(static_cast<int>(347 * scale), static_cast<int>(55 * scale), loadButtonWidth, loadButtonHeight);
    
    // Save button: using base dimensions
    saveButton.setBounds(static_cast<int>(415 * scale), static_cast<int>(55 * scale), loadButtonWidth, loadButtonHeight);
    
    // Distortion module knobs (both 160x160 in Figma, scaled to 80x80)
    int distortKnobSize = static_cast<int>(80 * scale); // Base: 80px (was 160 * 0.5)
    
    // DRIVE knob: 291 x 1969 -> adjusted: (466, 707) -> scaled: (233, 353.5)
    distortDriveSlider.setBounds(static_cast<int>(233 * scale), static_cast<int>(354 * scale), distortKnobSize, distortKnobSize);
    
    // TONE knob: 291 x 2202 -> adjusted: (466, 940) -> scaled: (233, 470)
    distortToneSlider.setBounds(static_cast<int>(233 * scale), static_cast<int>(470 * scale), distortKnobSize, distortKnobSize);
    
    // Crunch module switches - adjusted coordinates for current window size
    // ON switch: 212 x 264 (fine-tuned position) -> down 2 pixels
    distortOnButton.setBounds(static_cast<int>(212 * scale), static_cast<int>(266 * scale), switchWidth, switchHeight);
    
    // Pre/Post switch: 290 x 265 (fine-tuned position) -> left 2 pixels
    distortPrePostButton.setBounds(static_cast<int>(288 * scale), static_cast<int>(265 * scale), switchWidth, switchHeight);
    
    // Type selector: using base dimensions
    int selectorWidth = static_cast<int>(137 * scale); // Base: 137px (was 274 * 0.5)
    int selectorHeight = static_cast<int>(29 * scale); // Base: 29px (was 58 * 0.5)
    distortTypeSelector.setBounds(static_cast<int>(200 * scale), static_cast<int>(218 * scale), selectorWidth, selectorHeight);
    
    // Magnetic module knobs (all 100x100 in Figma, scaled to 50x50)
    int magneticKnobSize = static_cast<int>(50 * scale); // Base: 50px (was 100 * 0.5)
    
    // Comp knob: 1021 x 1931 -> adjusted: (1196, 669) -> scaled: (598, 334.5)
    magneticCompSlider.setBounds(static_cast<int>(598 * scale), static_cast<int>(335 * scale), magneticKnobSize, magneticKnobSize);
    
    // Sat knob: 943 x 2094 -> adjusted: (1118, 832) -> scaled: (559, 416)
    magneticSatSlider.setBounds(static_cast<int>(559 * scale), static_cast<int>(416 * scale), magneticKnobSize, magneticKnobSize);
    
    // X Talk knob: 1104 x 2094 -> adjusted: (1279, 832) -> scaled: (639.5, 416)
    magneticXTalkSlider.setBounds(static_cast<int>(640 * scale), static_cast<int>(416 * scale), magneticKnobSize, magneticKnobSize);
    
    // Head knob: 943 x 2262 -> adjusted: (1118, 1000) -> scaled: (559, 500)
    magneticHeadSlider.setBounds(static_cast<int>(559 * scale), static_cast<int>(500 * scale), magneticKnobSize, magneticKnobSize);
    
    // Wear knob: 1104 x 2262 -> adjusted: (1279, 1000) -> scaled: (639.5, 500)
    magneticWearSlider.setBounds(static_cast<int>(640 * scale), static_cast<int>(500 * scale), magneticKnobSize, magneticKnobSize);
    
    // Magnetic module switch (standard size)
    // ON switch: 1037 x 1738 -> adjusted: (1212, 476) -> scaled: (606, 238) -> left 1 pixel
    magneticOnButton.setBounds(static_cast<int>(606 * scale), static_cast<int>(238 * scale), switchWidth, switchHeight);
    
    // Atmosphere module knobs (formerly Noise - all 100x100 in Figma, scaled to 50x50)
    int noiseKnobSize = static_cast<int>(50 * scale); // Base: 50px (was 100 * 0.5)
    
    // Level knob: 1299 x 2014 -> adjusted: (1474, 752) -> scaled: (737, 376)
    noiseLevelSlider.setBounds(static_cast<int>(737 * scale), static_cast<int>(376 * scale), noiseKnobSize, noiseKnobSize);
    
    // Age knob: 1460 x 2014 -> adjusted: (1635, 752) -> scaled: (817.5, 376)
    noiseAgeSlider.setBounds(static_cast<int>(818 * scale), static_cast<int>(376 * scale), noiseKnobSize, noiseKnobSize);
    
    // Flutter knob: 1299 x 2182 -> adjusted: (1474, 920) -> scaled: (737, 460)
    noiseFlutterSlider.setBounds(static_cast<int>(737 * scale), static_cast<int>(460 * scale), noiseKnobSize, noiseKnobSize);
    
    // Width knob: 1460 x 2182 -> adjusted: (1635, 920) -> scaled: (817.5, 460)
    noiseWidthSlider.setBounds(static_cast<int>(818 * scale), static_cast<int>(460 * scale), noiseKnobSize, noiseKnobSize);
    
    // Atmosphere module switches and selector (formerly Noise)
    // ON switch: 1312 x 1795 -> adjusted: (1487, 533) -> scaled: (743.5, 266.5) -> up 2 pixels
    noiseOnButton.setBounds(static_cast<int>(744 * scale), static_cast<int>(265 * scale), switchWidth, switchHeight);
    
    // Pre/Post switch: 1460 x 1795 -> adjusted: (1635, 533) -> scaled: (817.5, 266.5) -> up 2 pixels
    noisePrePostButton.setBounds(static_cast<int>(818 * scale), static_cast<int>(265 * scale), switchWidth, switchHeight);
    
    // Type selector: using base dimensions
    int noiseTypeWidth = static_cast<int>(137 * scale); // Base: 137px (same as distortion selector)
    int noiseTypeHeight = static_cast<int>(29 * scale); // Base: 29px
    noiseTypeSelector.setBounds(static_cast<int>(730 * scale), static_cast<int>(218 * scale), noiseTypeWidth, noiseTypeHeight);
    
    // Type display label positioned inside the selector box
    noiseTypeLabel.setBounds(static_cast<int>(730 * scale), static_cast<int>(218 * scale), noiseTypeWidth, noiseTypeHeight);
    
    // Verb module knobs (formerly Space - all 100x100 in Figma, scaled to 50x50)
    int spaceKnobSize = static_cast<int>(50 * scale); // Base: 50px (was 100 * 0.5)
    
    // Mix knob: 1733 x 1931 -> adjusted: (1908, 669) -> scaled: (954, 334.5)
    spaceMixSlider.setBounds(static_cast<int>(954 * scale), static_cast<int>(335 * scale), spaceKnobSize, spaceKnobSize);
    
    // Time knob: 1655 x 2094 -> adjusted: (1830, 832) -> scaled: (915, 416)
    spaceTimeSlider.setBounds(static_cast<int>(915 * scale), static_cast<int>(416 * scale), spaceKnobSize, spaceKnobSize);
    
    // Tone knob: 1816 x 2094 -> adjusted: (1991, 832) -> scaled: (995.5, 416)
    spaceToneSlider.setBounds(static_cast<int>(996 * scale), static_cast<int>(416 * scale), spaceKnobSize, spaceKnobSize);
    
    // Pre-delay knob: 1655 x 2262 -> adjusted: (1830, 1000) -> scaled: (915, 500)
    spacePreDelaySlider.setBounds(static_cast<int>(915 * scale), static_cast<int>(500 * scale), spaceKnobSize, spaceKnobSize);
    
    // Cheapo knob: 1816 x 2262 -> adjusted: (1991, 1000) -> scaled: (995.5, 500)
    spaceCheapoSlider.setBounds(static_cast<int>(996 * scale), static_cast<int>(500 * scale), spaceKnobSize, spaceKnobSize);
    
    // Verb module switch (formerly Space)
    // ON switch: 1747 x 1738 -> adjusted: (1922, 476) -> scaled: (961, 238) -> left 1 pixel
    spaceOnButton.setBounds(static_cast<int>(961 * scale), static_cast<int>(238 * scale), switchWidth, switchHeight);
}

void ReallyCheapTwentyAudioProcessorEditor::setupMainKnobs()
{
    // Create invisible sliders that handle interaction, SVG knobs drawn on top
    
    // Input gain - invisible slider
    inGainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    inGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    inGainSlider.setRange(-24.0, 24.0, 0.1);
    inGainSlider.setValue(0.0);
    // Make completely transparent
    inGainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    inGainSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    inGainSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    inGainSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(inGainSlider);
    
    // Output gain - invisible slider
    outGainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    outGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    outGainSlider.setRange(-24.0, 24.0, 0.1);
    outGainSlider.setValue(0.0);
    outGainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    outGainSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    outGainSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    outGainSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(outGainSlider);
    
    // Mix - invisible slider
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixSlider.setRange(0.0, 1.0, 0.01);
    mixSlider.setValue(1.0);
    mixSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    mixSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    mixSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    mixSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(mixSlider);
    
    // Macro - invisible slider
    macroSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    macroSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    macroSlider.setRange(0.0, 1.0, 0.001);
    macroSlider.setValue(0.3);
    macroSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    macroSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    macroSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    macroSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(macroSlider);
    
    // Add value change listeners to trigger repaints
    inGainSlider.onValueChange = [this]() { repaint(); };
    outGainSlider.onValueChange = [this]() { repaint(); };
    mixSlider.onValueChange = [this]() { repaint(); };
    macroSlider.onValueChange = [this]() { repaint(); };
    
    // Create attachments
    inGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "inGain", inGainSlider);
    outGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "outGain", outGainSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "mix", mixSlider);
    macroAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "macroReallyCheap", macroSlider);
}

void ReallyCheapTwentyAudioProcessorEditor::setupWobbleKnobs()
{
    // Create invisible sliders that handle interaction, SVG knobs drawn on top
    
    // Wobble Depth
    wobbleDepthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wobbleDepthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    wobbleDepthSlider.setRange(0.0, 1.0, 0.01);
    wobbleDepthSlider.setValue(0.5);
    wobbleDepthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    wobbleDepthSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    wobbleDepthSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    wobbleDepthSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleDepthSlider);
    
    // Wobble Rate
    wobbleRateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wobbleRateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    wobbleRateSlider.setRange(0.1, 12.0, 0.1);
    wobbleRateSlider.setValue(1.2);
    wobbleRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    wobbleRateSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    wobbleRateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    wobbleRateSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleRateSlider);
    
    // Wobble Flutter
    wobbleFlutterSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wobbleFlutterSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    wobbleFlutterSlider.setRange(0.0, 1.0, 0.01);
    wobbleFlutterSlider.setValue(0.5);
    wobbleFlutterSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    wobbleFlutterSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    wobbleFlutterSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    wobbleFlutterSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleFlutterSlider);
    
    // Wobble Drift
    wobbleDriftSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wobbleDriftSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    wobbleDriftSlider.setRange(0.0, 1.0, 0.01);
    wobbleDriftSlider.setValue(0.5);
    wobbleDriftSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    wobbleDriftSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    wobbleDriftSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    wobbleDriftSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleDriftSlider);
    
    // Wobble Jitter
    wobbleJitterSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wobbleJitterSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    wobbleJitterSlider.setRange(0.0, 1.0, 0.01);
    wobbleJitterSlider.setValue(0.5);
    wobbleJitterSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    wobbleJitterSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    wobbleJitterSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    wobbleJitterSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleJitterSlider);
    
    // Wobble Link
    wobbleLinkSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wobbleLinkSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    wobbleLinkSlider.setRange(0.0, 1.0, 0.01);
    wobbleLinkSlider.setValue(0.5);
    wobbleLinkSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    wobbleLinkSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    wobbleLinkSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    wobbleLinkSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleLinkSlider);
    
    // Add value change listeners to trigger repaints
    wobbleDepthSlider.onValueChange = [this]() { repaint(); };
    wobbleRateSlider.onValueChange = [this]() { repaint(); };
    wobbleFlutterSlider.onValueChange = [this]() { repaint(); };
    wobbleDriftSlider.onValueChange = [this]() { repaint(); };
    wobbleJitterSlider.onValueChange = [this]() { repaint(); };
    wobbleLinkSlider.onValueChange = [this]() { repaint(); };
    
    // Create attachments to actual parameters
    wobbleDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "wobbleDepth", wobbleDepthSlider);
    wobbleRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "wobbleRateHz", wobbleRateSlider);
    wobbleFlutterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "wobbleFlutter", wobbleFlutterSlider);
    wobbleDriftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "wobbleDrift", wobbleDriftSlider);
    wobbleJitterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "wobbleJitter", wobbleJitterSlider);
    wobbleLinkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "wobbleStereoLink", wobbleLinkSlider);
}

void ReallyCheapTwentyAudioProcessorEditor::setupWobbleSwitches()
{
    // Create invisible toggle buttons that handle interaction, SVG switches drawn on top
    
    // Wobble ON switch
    wobbleOnButton.setButtonText("");
    wobbleOnButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    wobbleOnButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    wobbleOnButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleOnButton);
    
    // Wobble SYNC switch
    wobbleSyncButton.setButtonText("");
    wobbleSyncButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    wobbleSyncButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    wobbleSyncButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleSyncButton);
    
    // Wobble MONO switch
    wobbleMonoButton.setButtonText("");
    wobbleMonoButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    wobbleMonoButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    wobbleMonoButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(wobbleMonoButton);
    
    // Add value change listeners to trigger repaints
    wobbleOnButton.onStateChange = [this]() { repaint(); };
    wobbleSyncButton.onStateChange = [this]() { repaint(); };
    wobbleMonoButton.onStateChange = [this]() { repaint(); };
    
    // Create attachments to actual parameters
    wobbleOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "wobbleOn", wobbleOnButton);
    wobbleSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "wobbleSync", wobbleSyncButton);
    wobbleMonoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "wobbleMono", wobbleMonoButton);
}

void ReallyCheapTwentyAudioProcessorEditor::setupGlobalControls()
{
    // Clean JUCE controls for testing - will position over SVG later
    
    // Title 
    titleLabel.setText("ReallyChea🛒 Twenty™", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);
    
    // Input gain
    inGainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    inGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    inGainSlider.setRange(-24.0, 24.0, 0.1);
    inGainSlider.setValue(0.0);
    addAndMakeVisible(inGainSlider);
    
    inGainLabel.setText("Input", juce::dontSendNotification);
    inGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(inGainLabel);
    
    // Output gain
    outGainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    outGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    outGainSlider.setRange(-24.0, 24.0, 0.1);
    outGainSlider.setValue(0.0);
    addAndMakeVisible(outGainSlider);
    
    outGainLabel.setText("Output", juce::dontSendNotification);
    outGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outGainLabel);
    
    // Mix
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    mixSlider.setRange(0.0, 1.0, 0.01);
    mixSlider.setValue(1.0);
    addAndMakeVisible(mixSlider);
    
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);
    
    // Bypass
    bypassButton.setButtonText("Bypass");
    addAndMakeVisible(bypassButton);
    
    bypassLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(bypassLabel);
    
    // Macro
    macroSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    macroSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    macroSlider.setRange(0.0, 1.0, 0.001);
    macroSlider.setValue(0.3);
    addAndMakeVisible(macroSlider);
    
    macroLabel.setText("MACRO", juce::dontSendNotification);
    macroLabel.setJustificationType(juce::Justification::centred);
    macroLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(macroLabel);
    
    // Create bypass attachment (others created in setupMainKnobs)
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "bypass", bypassButton);
}

void ReallyCheapTwentyAudioProcessorEditor::setupPresetControls()
{
    // Clean JUCE preset controls for testing
    
    // Preset label - hidden since we use SVG
    presetLabel.setText("Preset:", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    // addAndMakeVisible(presetLabel); // Hidden for SVG UI
    
    // Preset combo box - make completely transparent so only SVG shows
    presetComboBox.setTextWhenNoChoicesAvailable("");
    presetComboBox.setTextWhenNothingSelected("");
    presetComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    presetComboBox.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    presetComboBox.setColour(juce::ComboBox::buttonColourId, juce::Colours::transparentBlack);
    presetComboBox.setColour(juce::ComboBox::arrowColourId, juce::Colours::transparentBlack);
    presetComboBox.setColour(juce::ComboBox::textColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(presetComboBox);
    
    // Previous/Next buttons - hidden since we use SVG dropdown
    prevButton.setButtonText("<");
    prevButton.setTooltip("Previous preset");
    // addAndMakeVisible(prevButton); // Hidden for SVG UI
    
    nextButton.setButtonText(">");
    nextButton.setTooltip("Next preset");
    // addAndMakeVisible(nextButton); // Hidden for SVG UI
    
    // Save/Load buttons - invisible but positioned over SVG buttons
    saveButton.setButtonText("");
    saveButton.setTooltip("Save current settings as user preset");
    saveButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    saveButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    saveButton.setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
    saveButton.setColour(juce::TextButton::textColourOnId, juce::Colours::transparentBlack);
    // Add custom property to identify as preset button
    saveButton.getProperties().set("isPresetButton", true);
    addAndMakeVisible(saveButton);
    
    loadButton.setButtonText("");
    loadButton.setTooltip("Load the selected preset");
    loadButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    loadButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    loadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
    loadButton.setColour(juce::TextButton::textColourOnId, juce::Colours::transparentBlack);
    // Add custom property to identify as preset button
    loadButton.getProperties().set("isPresetButton", true);
    addAndMakeVisible(loadButton);
    
    // Set up callbacks
    presetComboBox.onChange = [this] { /* Don't auto-load, just update display */ repaint(); };
    prevButton.onClick = [this] { prevPresetClicked(); };
    nextButton.onClick = [this] { nextPresetClicked(); };
    saveButton.onClick = [this] { savePresetClicked(); };
    loadButton.onClick = [this] { loadSelectedPreset(); };
    
    // Initialize preset list
    updatePresetComboBox();
}

void ReallyCheapTwentyAudioProcessorEditor::updatePresetComboBox()
{
    presetComboBox.clear();
    
    // Add factory presets
    auto factoryPresets = audioProcessor.getPresetManager().getFactoryPresetNames();
    for (int i = 0; i < factoryPresets.size(); ++i)
    {
        presetComboBox.addItem(factoryPresets[i], i + 1);
    }
    
    // Add separator
    if (!factoryPresets.isEmpty())
        presetComboBox.addSeparator();
    
    // Add user presets
    auto userPresets = audioProcessor.getPresetManager().getUserPresetNames();
    for (int i = 0; i < userPresets.size(); ++i)
    {
        presetComboBox.addItem("U: " + userPresets[i], factoryPresets.size() + i + 2);
    }
}

void ReallyCheapTwentyAudioProcessorEditor::savePresetClicked()
{
    // Open dialog to get preset name
    juce::AlertWindow::showAsync(
        juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::NoIcon)
            .withTitle("Save Preset")
            .withMessage("Enter preset name:")
            .withButton("Save")
            .withButton("Cancel"),
        [this](int result)
        {
            if (result == 1) // Save button
            {
                // Get the text from the alert window - this is a simplified approach
                // In a real implementation, you'd use a custom dialog with a text input
                auto presetName = "New Preset " + juce::String(juce::Random::getSystemRandom().nextInt(1000));
                
                if (audioProcessor.getPresetManager().saveUserPreset(presetName))
                {
                    updatePresetComboBox();
                    
                    // Select the newly saved preset
                    auto userPresets = audioProcessor.getPresetManager().getUserPresetNames();
                    auto factoryCount = audioProcessor.getPresetManager().getFactoryPresetNames().size();
                    auto index = userPresets.indexOf(presetName);
                    if (index >= 0)
                    {
                        presetComboBox.setSelectedId(factoryCount + index + 2);
                    }
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Save Failed",
                        "Could not save preset. Please try again.");
                }
            }
        }
    );
}

void ReallyCheapTwentyAudioProcessorEditor::loadPresetClicked()
{
    auto& presetManager = audioProcessor.getPresetManager();
    auto userDir = presetManager.getUserPresetsDirectory();
    
    auto chooser = std::make_unique<juce::FileChooser>("Load Preset",
                                                       userDir,
                                                       "*.rc20preset");
    
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;
                      
    chooser->launchAsync(chooserFlags, [this, &presetManager](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            if (presetManager.loadPreset(file))
            {
                updatePresetComboBox();
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Load Failed",
                    "Could not load preset file: " + file.getFileName());
            }
        }
    });
}

void ReallyCheapTwentyAudioProcessorEditor::prevPresetClicked()
{
    if (audioProcessor.getPresetManager().loadPreviousPreset())
    {
        auto currentIndex = audioProcessor.getPresetManager().getCurrentPresetIndex();
        auto allPresets = audioProcessor.getPresetManager().getAllPresetNames();
        
        if (currentIndex >= 0 && currentIndex < allPresets.size())
        {
            // Update combo box selection
            auto factoryCount = audioProcessor.getPresetManager().getFactoryPresetNames().size();
            if (currentIndex < factoryCount)
                presetComboBox.setSelectedId(currentIndex + 1);
            else
                presetComboBox.setSelectedId(currentIndex + 2); // +2 for separator
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::nextPresetClicked()
{
    if (audioProcessor.getPresetManager().loadNextPreset())
    {
        auto currentIndex = audioProcessor.getPresetManager().getCurrentPresetIndex();
        auto allPresets = audioProcessor.getPresetManager().getAllPresetNames();
        
        if (currentIndex >= 0 && currentIndex < allPresets.size())
        {
            // Update combo box selection
            auto factoryCount = audioProcessor.getPresetManager().getFactoryPresetNames().size();
            if (currentIndex < factoryCount)
                presetComboBox.setSelectedId(currentIndex + 1);
            else
                presetComboBox.setSelectedId(currentIndex + 2); // +2 for separator
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::presetComboChanged()
{
    // No longer auto-loads presets - just updates the display
    repaint();
}

void ReallyCheapTwentyAudioProcessorEditor::loadSelectedPreset()
{
    auto selectedId = presetComboBox.getSelectedId();
    if (selectedId == 0) return;
    
    auto& presetManager = audioProcessor.getPresetManager();
    auto factoryPresets = presetManager.getFactoryPresetNames();
    auto userPresets = presetManager.getUserPresetNames();
    
    if (selectedId <= factoryPresets.size())
    {
        // Factory preset
        auto presetName = factoryPresets[selectedId - 1];
        presetManager.loadFactoryPreset(presetName);
        presetManager.setCurrentPresetIndex(selectedId - 1);
    }
    else
    {
        // User preset (account for separator)
        auto userIndex = selectedId - factoryPresets.size() - 2;
        if (userIndex >= 0 && userIndex < userPresets.size())
        {
            auto presetName = userPresets[userIndex];
            presetManager.loadUserPreset(presetName);
            presetManager.setCurrentPresetIndex(factoryPresets.size() + userIndex);
        }
    }
    
    // Update display to show current preset name
    repaint();
}

void ReallyCheapTwentyAudioProcessorEditor::loadBackgroundSVG()
{
    // Try multiple locations for the SVG background
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets").getChildFile("FULL PLUGIN MOCKUP.svg"));
    possiblePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("ReallyCheap-Twenty SVG assets").getChildFile("FULL PLUGIN MOCKUP.svg"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets\\FULL PLUGIN MOCKUP.svg"));
    
    for (auto& path : possiblePaths)
    {
        if (path.existsAsFile())
        {
            backgroundSVG = juce::Drawable::createFromSVGFile(path);
            if (backgroundSVG != nullptr)
            {
                DBG("Loaded background SVG from: " << path.getFullPathName());
                return;
            }
        }
    }
    
    DBG("Could not load background SVG - using fallback color");
}

void ReallyCheapTwentyAudioProcessorEditor::loadKnobSVGs()
{
    // Load individual knob SVGs
    juce::Array<juce::File> basePaths;
    basePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    basePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    basePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : basePaths)
    {
        // Input Gain knob
        auto inputGainFile = basePath.getChildFile("Knob (input gain).svg");
        if (inputGainFile.existsAsFile() && inputGainKnobSVG == nullptr)
        {
            inputGainKnobSVG = juce::Drawable::createFromSVGFile(inputGainFile);
            if (inputGainKnobSVG != nullptr)
                DBG("Loaded input gain knob SVG");
        }
        
        // Output Gain knob
        auto outputGainFile = basePath.getChildFile("Knob (output gain).svg");
        if (outputGainFile.existsAsFile() && outputGainKnobSVG == nullptr)
        {
            outputGainKnobSVG = juce::Drawable::createFromSVGFile(outputGainFile);
            if (outputGainKnobSVG != nullptr)
                DBG("Loaded output gain knob SVG");
        }
        
        // Mix knob
        auto mixFile = basePath.getChildFile("Knob (mix).svg");
        if (mixFile.existsAsFile() && mixKnobSVG == nullptr)
        {
            mixKnobSVG = juce::Drawable::createFromSVGFile(mixFile);
            if (mixKnobSVG != nullptr)
                DBG("Loaded mix knob SVG");
        }
        
        // Macro knob
        auto macroFile = basePath.getChildFile("Knob (reallycheap macro).svg");
        if (macroFile.existsAsFile() && macroKnobSVG == nullptr)
        {
            macroKnobSVG = juce::Drawable::createFromSVGFile(macroFile);
            if (macroKnobSVG != nullptr)
                DBG("Loaded macro knob SVG");
        }
        
        // Break if all knob assets loaded
        if (inputGainKnobSVG && outputGainKnobSVG && mixKnobSVG && macroKnobSVG)
            break;
    }
    
    DBG("Knob SVG loading complete");
}

void ReallyCheapTwentyAudioProcessorEditor::loadTitleCardImage()
{
    // Safely load title card from embedded binary data
    try
    {
        auto titleCardData = BinaryData::titlecard_png;
        auto titleCardSize = BinaryData::titlecard_pngSize;
        
        if (titleCardData != nullptr && titleCardSize > 0)
        {
            titleCardImage = juce::ImageFileFormat::loadFrom(titleCardData, titleCardSize);
            
            if (titleCardImage.isValid())
            {
                DBG("Successfully loaded title card from embedded data");
                DBG("Title card dimensions: " << titleCardImage.getWidth() << "x" << titleCardImage.getHeight());
            }
            else
            {
                DBG("Failed to decode title card from embedded data");
            }
        }
        else
        {
            DBG("Title card binary data is null or empty");
        }
    }
    catch (...)
    {
        DBG("Exception occurred while loading title card - continuing without it");
        titleCardImage = juce::Image();
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadWobbleKnobSVGs()
{
    // Load individual wobble knob SVGs
    juce::Array<juce::File> basePaths;
    basePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    basePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    basePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : basePaths)
    {
        // Wobble Depth knob
        auto wobbleDepthFile = basePath.getChildFile("Knob Small (wobble - depth).svg");
        if (wobbleDepthFile.existsAsFile() && wobbleDepthKnobSVG == nullptr)
        {
            wobbleDepthKnobSVG = juce::Drawable::createFromSVGFile(wobbleDepthFile);
            if (wobbleDepthKnobSVG != nullptr)
                DBG("Loaded wobble depth knob SVG");
        }
        
        // Wobble Rate knob  
        auto wobbleRateFile = basePath.getChildFile("Knob Small (wobble - rate).svg");
        if (wobbleRateFile.existsAsFile() && wobbleRateKnobSVG == nullptr)
        {
            wobbleRateKnobSVG = juce::Drawable::createFromSVGFile(wobbleRateFile);
            if (wobbleRateKnobSVG != nullptr)
                DBG("Loaded wobble rate knob SVG");
        }
        
        // Wobble Flutter knob
        auto wobbleFlutterFile = basePath.getChildFile("Knob Small (wobble - flutter).svg");
        if (wobbleFlutterFile.existsAsFile() && wobbleFlutterKnobSVG == nullptr)
        {
            wobbleFlutterKnobSVG = juce::Drawable::createFromSVGFile(wobbleFlutterFile);
            if (wobbleFlutterKnobSVG != nullptr)
                DBG("Loaded wobble flutter knob SVG");
        }
        
        // Wobble Drift knob
        auto wobbleDriftFile = basePath.getChildFile("Knob Small (wobble - drift).svg");
        if (wobbleDriftFile.existsAsFile() && wobbleDriftKnobSVG == nullptr)
        {
            wobbleDriftKnobSVG = juce::Drawable::createFromSVGFile(wobbleDriftFile);
            if (wobbleDriftKnobSVG != nullptr)
                DBG("Loaded wobble drift knob SVG");
        }
        
        // Wobble Jitter knob
        auto wobbleJitterFile = basePath.getChildFile("Knob Small (wobble - jitter).svg");
        if (wobbleJitterFile.existsAsFile() && wobbleJitterKnobSVG == nullptr)
        {
            wobbleJitterKnobSVG = juce::Drawable::createFromSVGFile(wobbleJitterFile);
            if (wobbleJitterKnobSVG != nullptr)
                DBG("Loaded wobble jitter knob SVG");
        }
        
        // Wobble Link knob
        auto wobbleLinkFile = basePath.getChildFile("Knob Small (wobble - link).svg");
        if (wobbleLinkFile.existsAsFile() && wobbleLinkKnobSVG == nullptr)
        {
            wobbleLinkKnobSVG = juce::Drawable::createFromSVGFile(wobbleLinkFile);
            if (wobbleLinkKnobSVG != nullptr)
                DBG("Loaded wobble link knob SVG");
        }
        
        // Break if all wobble knob assets loaded
        if (wobbleDepthKnobSVG && wobbleRateKnobSVG && wobbleFlutterKnobSVG && 
            wobbleDriftKnobSVG && wobbleJitterKnobSVG && wobbleLinkKnobSVG)
            break;
    }
    
    DBG("Wobble knob SVG loading complete");
}

void ReallyCheapTwentyAudioProcessorEditor::loadWobbleSwitchSVGs()
{
    // Load wobble switch SVGs
    juce::Array<juce::File> basePaths;
    basePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    basePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    basePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : basePaths)
    {
        // Switch OFF state (shared by all switches when off)
        auto switchOffFile = basePath.getChildFile("SWITCH OFF.svg");
        if (switchOffFile.existsAsFile() && switchOffSVG == nullptr)
        {
            switchOffSVG = juce::Drawable::createFromSVGFile(switchOffFile);
            if (switchOffSVG != nullptr)
                DBG("Loaded switch OFF SVG");
        }
        
        // Wobble ON switch
        auto wobbleOnFile = basePath.getChildFile("WOB SWITCH ON.svg");
        if (wobbleOnFile.existsAsFile() && wobbleOnSwitchSVG == nullptr)
        {
            wobbleOnSwitchSVG = juce::Drawable::createFromSVGFile(wobbleOnFile);
            if (wobbleOnSwitchSVG != nullptr)
                DBG("Loaded wobble ON switch SVG");
        }
        
        // Wobble SYNC switch
        auto wobbleSyncFile = basePath.getChildFile("WOB SWITCH SYNC.svg");
        if (wobbleSyncFile.existsAsFile() && wobbleSyncSwitchSVG == nullptr)
        {
            wobbleSyncSwitchSVG = juce::Drawable::createFromSVGFile(wobbleSyncFile);
            if (wobbleSyncSwitchSVG != nullptr)
                DBG("Loaded wobble SYNC switch SVG");
        }
        
        // Wobble MONO switch
        auto wobbleMonoFile = basePath.getChildFile("WOB SWITCH MONO.svg");
        if (wobbleMonoFile.existsAsFile() && wobbleMonoSwitchSVG == nullptr)
        {
            wobbleMonoSwitchSVG = juce::Drawable::createFromSVGFile(wobbleMonoFile);
            if (wobbleMonoSwitchSVG != nullptr)
                DBG("Loaded wobble MONO switch SVG");
        }
        
        // Break if all switch assets loaded
        if (switchOffSVG && wobbleOnSwitchSVG && wobbleSyncSwitchSVG && wobbleMonoSwitchSVG)
            break;
    }
    
    DBG("Wobble switch SVG loading complete");
}

void ReallyCheapTwentyAudioProcessorEditor::drawWobbleSwitches(juce::Graphics& g)
{
    // Helper function to draw a switch with on/off states
    auto drawSwitch = [&](juce::ToggleButton& button, std::unique_ptr<juce::Drawable>& onSwitchSVG)
    {
        auto bounds = button.getBounds().toFloat();
        
        if (button.getToggleState() && onSwitchSVG != nullptr)
        {
            // Draw ON state
            onSwitchSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else if (switchOffSVG != nullptr)
        {
            // Draw OFF state
            switchOffSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    };
    
    // Draw all wobble switches
    drawSwitch(wobbleOnButton, wobbleOnSwitchSVG);
    drawSwitch(wobbleSyncButton, wobbleSyncSwitchSVG);
    drawSwitch(wobbleMonoButton, wobbleMonoSwitchSVG);
}

void ReallyCheapTwentyAudioProcessorEditor::drawWobbleKnobs(juce::Graphics& g)
{
    // Helper function to draw a wobble knob with rotation
    auto drawWobbleKnob = [&](juce::Slider& slider, std::unique_ptr<juce::Drawable>& knobSVG)
    {
        if (knobSVG != nullptr)
        {
            auto bounds = slider.getBounds().toFloat();
            auto centerX = bounds.getCentreX();
            auto centerY = bounds.getCentreY();
            
            // Calculate rotation angle based on slider value (-150° to +150°)
            auto normalizedValue = (slider.getValue() - slider.getMinimum()) / 
                                  (slider.getMaximum() - slider.getMinimum());
            auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
            
            g.saveState();
            g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
            knobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
            g.restoreState();
        }
    };
    
    // Draw all wobble knobs
    drawWobbleKnob(wobbleDepthSlider, wobbleDepthKnobSVG);
    drawWobbleKnob(wobbleRateSlider, wobbleRateKnobSVG);
    drawWobbleKnob(wobbleFlutterSlider, wobbleFlutterKnobSVG);
    drawWobbleKnob(wobbleDriftSlider, wobbleDriftKnobSVG);
    drawWobbleKnob(wobbleJitterSlider, wobbleJitterKnobSVG);
    drawWobbleKnob(wobbleLinkSlider, wobbleLinkKnobSVG);
}

void ReallyCheapTwentyAudioProcessorEditor::setupDigitalKnobs()
{
    // Create invisible sliders that handle interaction, SVG knobs drawn on top
    
    // Digital Bits
    digitalBitsSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    digitalBitsSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    digitalBitsSlider.setRange(4, 16, 1);
    digitalBitsSlider.setValue(12);
    digitalBitsSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    digitalBitsSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    digitalBitsSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    digitalBitsSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(digitalBitsSlider);
    
    // Digital Sample Rate
    digitalSRSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    digitalSRSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    digitalSRSlider.setRange(6000.0, 44100.0, 100.0);
    digitalSRSlider.setValue(44100.0);
    digitalSRSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    digitalSRSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    digitalSRSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    digitalSRSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(digitalSRSlider);
    
    // Digital Jitter
    digitalJitterSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    digitalJitterSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    digitalJitterSlider.setRange(0.0, 1.0, 0.01);
    digitalJitterSlider.setValue(0.0);
    digitalJitterSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    digitalJitterSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    digitalJitterSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    digitalJitterSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(digitalJitterSlider);
    
    // Add repaint callbacks for visual feedback
    digitalBitsSlider.onValueChange = [this]() { repaint(); };
    digitalSRSlider.onValueChange = [this]() { repaint(); };
    digitalJitterSlider.onValueChange = [this]() { repaint(); };
    
    // Create attachments to actual parameters
    digitalBitsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "digitalBits", digitalBitsSlider);
    digitalSRAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "digitalSR", digitalSRSlider);
    digitalJitterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "digitalJitter", digitalJitterSlider);
}

void ReallyCheapTwentyAudioProcessorEditor::setupDigitalSwitches()
{
    // Create invisible toggle buttons that handle interaction, SVG switches drawn on top
    
    // Digital ON switch
    digitalOnButton.setButtonText("");
    digitalOnButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    digitalOnButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    digitalOnButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    digitalOnButton.onStateChange = [this]() { repaint(); };
    addAndMakeVisible(digitalOnButton);
    
    // Digital AA switch  
    digitalAAButton.setButtonText("");
    digitalAAButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    digitalAAButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    digitalAAButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    digitalAAButton.onStateChange = [this]() { repaint(); };
    addAndMakeVisible(digitalAAButton);
    
    // Create attachments to actual parameters
    digitalOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "digitalOn", digitalOnButton);
    digitalAAAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "digitalAA", digitalAAButton);
}

void ReallyCheapTwentyAudioProcessorEditor::loadDigitalKnobSVGs()
{
    // Base paths to try for SVG assets
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load Digital knob SVGs with correct file names
        auto bitsKnobFile = basePath.getChildFile("Knob Small (digital -bits).svg");
        if (bitsKnobFile.existsAsFile() && digitalBitsKnobSVG == nullptr)
        {
            digitalBitsKnobSVG = juce::Drawable::createFromSVGFile(bitsKnobFile);
            if (digitalBitsKnobSVG != nullptr)
                DBG("Loaded bitcrush bits knob SVG (formerly digital)");
        }
        
        auto srKnobFile = basePath.getChildFile("Knob Small (digital - sample rate).svg");
        if (srKnobFile.existsAsFile() && digitalSRKnobSVG == nullptr)
        {
            digitalSRKnobSVG = juce::Drawable::createFromSVGFile(srKnobFile);
            if (digitalSRKnobSVG != nullptr)
                DBG("Loaded bitcrush SR knob SVG (formerly digital)");
        }
        
        auto jitterKnobFile = basePath.getChildFile("Knob Small (digital - jitter).svg");
        if (jitterKnobFile.existsAsFile() && digitalJitterKnobSVG == nullptr)
        {
            digitalJitterKnobSVG = juce::Drawable::createFromSVGFile(jitterKnobFile);
            if (digitalJitterKnobSVG != nullptr)
                DBG("Loaded bitcrush jitter knob SVG (formerly digital)");
        }
        
        // If we have all knobs, we're done
        if (digitalBitsKnobSVG != nullptr && digitalSRKnobSVG != nullptr && digitalJitterKnobSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadDigitalSwitchSVGs()
{
    // Base paths to try for SVG assets
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load Digital switch SVGs with correct file names
        auto onSwitchFile = basePath.getChildFile("DIG SWITCH ON.svg");
        if (onSwitchFile.existsAsFile() && digitalOnSwitchSVG == nullptr)
        {
            digitalOnSwitchSVG = juce::Drawable::createFromSVGFile(onSwitchFile);
            if (digitalOnSwitchSVG != nullptr)
                DBG("Loaded digital ON switch SVG");
        }
        
        auto aaSwitchFile = basePath.getChildFile("DIG SWITCH AA.svg");
        if (aaSwitchFile.existsAsFile() && digitalAASwitchSVG == nullptr)
        {
            digitalAASwitchSVG = juce::Drawable::createFromSVGFile(aaSwitchFile);
            if (digitalAASwitchSVG != nullptr)
                DBG("Loaded digital AA switch SVG");
        }
        
        // If we have all switches, we're done
        if (digitalOnSwitchSVG != nullptr && digitalAASwitchSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawDigitalKnobs(juce::Graphics& g)
{
    auto drawDigitalKnob = [&](juce::Slider& slider, std::unique_ptr<juce::Drawable>& knobSVG)
    {
        if (knobSVG != nullptr)
        {
            auto bounds = slider.getBounds().toFloat();
            auto centerX = bounds.getCentreX();
            auto centerY = bounds.getCentreY();
            
            // Calculate rotation angle based on slider value (-150° to +150°)
            auto normalizedValue = (slider.getValue() - slider.getMinimum()) / 
                                  (slider.getMaximum() - slider.getMinimum());
            auto rotationAngle = -150.0f + (normalizedValue * 300.0f); // -150° to +150°
            
            g.saveState();
            g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
            knobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
            g.restoreState();
        }
    };
    
    // Draw all digital knobs
    drawDigitalKnob(digitalBitsSlider, digitalBitsKnobSVG);
    drawDigitalKnob(digitalSRSlider, digitalSRKnobSVG);
    drawDigitalKnob(digitalJitterSlider, digitalJitterKnobSVG);
}

void ReallyCheapTwentyAudioProcessorEditor::drawDigitalSwitches(juce::Graphics& g)
{
    auto drawSwitch = [&](juce::ToggleButton& button, std::unique_ptr<juce::Drawable>& onSwitchSVG)
    {
        auto bounds = button.getBounds().toFloat();
        
        if (button.getToggleState() && onSwitchSVG != nullptr)
        {
            onSwitchSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else if (switchOffSVG != nullptr)
        {
            switchOffSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    };
    
    // Draw all digital switches
    drawSwitch(digitalOnButton, digitalOnSwitchSVG);
    drawSwitch(digitalAAButton, digitalAASwitchSVG);
}

void ReallyCheapTwentyAudioProcessorEditor::loadPresetSelectorSVGs()
{
    // Base paths to try for SVG assets
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load preset selector SVGs
        auto closedFile = basePath.getChildFile("Preset Selector Closed.svg");
        if (closedFile.existsAsFile() && presetSelectorClosedSVG == nullptr)
        {
            presetSelectorClosedSVG = juce::Drawable::createFromImageFile(closedFile);
        }
        
        auto openFile = basePath.getChildFile("Preset Selector Open.svg");
        if (openFile.existsAsFile() && presetSelectorOpenSVG == nullptr)
        {
            presetSelectorOpenSVG = juce::Drawable::createFromImageFile(openFile);
        }
        
        // If we have both, we're done
        if (presetSelectorClosedSVG != nullptr && presetSelectorOpenSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawPresetSelector(juce::Graphics& g)
{
    auto bounds = presetComboBox.getBounds().toFloat();
    
    // Always draw the closed state - when opened, JUCE handles the popup automatically
    if (presetSelectorClosedSVG != nullptr)
    {
        presetSelectorClosedSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
    }
    
    // Draw the current preset text on top of the SVG
    auto selectedText = presetComboBox.getText();
    if (selectedText.isNotEmpty())
    {
        g.setColour(juce::Colours::black);
        g.setFont(customFont.withHeight(16.0f)); // Increased from 12 to 16
        
        // Position text within the selector, slightly offset from left edge
        auto textBounds = bounds.reduced(12, 0); // Slightly more padding
        g.drawText(selectedText, textBounds.toNearestInt(), juce::Justification::centredLeft, true);
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadCustomFont()
{
    // Try to load custom font from assets folder
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets\\fonts"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets").getChildFile("fonts"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    juce::Font loadedFont(juce::FontOptions(12.0f));
    bool fontLoaded = false;
    
    for (auto& basePath : possiblePaths)
    {
        // Look for common font file extensions
        juce::Array<juce::String> fontExtensions = {"*.ttf", "*.otf", "*.TTF", "*.OTF"};
        
        for (auto& extension : fontExtensions)
        {
            auto fontFiles = basePath.findChildFiles(juce::File::findFiles, false, extension);
            
            if (!fontFiles.isEmpty())
            {
                // Try to load the first font file found
                auto fontFile = fontFiles[0];
                
                if (fontFile.existsAsFile())
                {
                    // Create font from file using JUCE's font loading
                    auto fontStream = fontFile.createInputStream();
                    if (fontStream != nullptr)
                    {
                        juce::MemoryBlock fontData;
                        fontStream->readIntoMemoryBlock(fontData);
                        
                        if (fontData.getSize() > 0)
                        {
                            auto typeface = juce::Typeface::createSystemTypefaceFor(fontData.getData(), fontData.getSize());
                            if (typeface.get() != nullptr)
                            {
                                loadedFont = juce::Font(juce::FontOptions().withTypeface(typeface));
                                fontLoaded = true;
                                DBG("Custom font loaded: " << fontFile.getFileName());
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        if (fontLoaded) break;
    }
    
    // Fallback to system fonts if no custom font found
    if (!fontLoaded)
    {
        // Try some common system fonts that might fit the aesthetic
        juce::StringArray systemFonts = {
            "Arial Black",      // Bold sans-serif
            "Impact",           // Bold condensed 
            "Helvetica Bold",   // Clean bold
            "Futura",          // Geometric
            "Bebas Neue",      // Modern condensed
            "Oswald",          // Modern sans-serif
            "Arial Bold",      // Fallback bold
            "Arial"            // Final fallback
        };
        
        for (auto& fontName : systemFonts)
        {
            juce::Font testFont(juce::FontOptions(fontName, 12.0f, juce::Font::bold));
            if (testFont.getTypefaceName() == fontName)
            {
                loadedFont = testFont;
                DBG("Using system font: " << fontName);
                break;
            }
        }
        
        // Ultimate fallback
        if (loadedFont.getTypefaceName().isEmpty())
        {
            loadedFont = juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 12.0f, juce::Font::bold));
            DBG("Using default font fallback");
        }
    }
    
    customFont = loadedFont;
}

void ReallyCheapTwentyAudioProcessorEditor::setupDistortKnobs()
{
    // DRIVE knob
    distortDriveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    distortDriveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    distortDriveSlider.setRange(0.0, 1.0, 0.01);
    distortDriveSlider.setValue(0.5);
    distortDriveSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    distortDriveSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    distortDriveSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    distortDriveSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(distortDriveSlider);
    
    // TONE knob
    distortToneSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    distortToneSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    distortToneSlider.setRange(0.0, 1.0, 0.01);
    distortToneSlider.setValue(0.5);
    distortToneSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    distortToneSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    distortToneSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    distortToneSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(distortToneSlider);
    
    // Add repaint callbacks for visual feedback (like wobble module)
    distortDriveSlider.onValueChange = [this]() { repaint(); };
    distortToneSlider.onValueChange = [this]() { repaint(); };
    
    // Create parameter attachments
    distortDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "distortDrive", distortDriveSlider);
    distortToneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "distortTone", distortToneSlider);
}

void ReallyCheapTwentyAudioProcessorEditor::setupDistortControls()
{
    // ON switch
    distortOnButton.setButtonText("");
    distortOnButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    distortOnButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    distortOnButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(distortOnButton);
    
    // Pre/Post switch
    distortPrePostButton.setButtonText("");
    distortPrePostButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    distortPrePostButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    distortPrePostButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(distortPrePostButton);
    
    // Type selector
    distortTypeSelector.setTextWhenNoChoicesAvailable("");
    distortTypeSelector.setTextWhenNothingSelected("");
    distortTypeSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    distortTypeSelector.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    distortTypeSelector.setColour(juce::ComboBox::buttonColourId, juce::Colours::transparentBlack);
    distortTypeSelector.setColour(juce::ComboBox::arrowColourId, juce::Colours::transparentBlack);
    distortTypeSelector.setColour(juce::ComboBox::textColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(distortTypeSelector);
    
    // Add distortion type options using the new combined type+oversampling choices
    for (int i = 0; i < ReallyCheap::ParameterHelper::getDistortTypeChoices().size(); ++i)
    {
        distortTypeSelector.addItem(ReallyCheap::ParameterHelper::getDistortTypeChoices()[i], i + 1);
    }
    
    // Add repaint callbacks for visual feedback  
    distortOnButton.onStateChange = [this]() { repaint(); };
    distortPrePostButton.onStateChange = [this]() { repaint(); };
    distortTypeSelector.onChange = [this]() { repaint(); };
    
    // Create parameter attachments
    distortOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "distortOn", distortOnButton);
    distortPrePostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "distortPrePost", distortPrePostButton);
    distortTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "distortType", distortTypeSelector);
}

void ReallyCheapTwentyAudioProcessorEditor::loadDistortKnobSVGs()
{
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load distortion knob SVGs with correct file names
        auto driveKnobFile = basePath.getChildFile("Knob (large) (distortion - drive).svg");
        if (driveKnobFile.existsAsFile() && distortDriveKnobSVG == nullptr)
        {
            distortDriveKnobSVG = juce::Drawable::createFromSVGFile(driveKnobFile);
            if (distortDriveKnobSVG != nullptr)
                DBG("Loaded distortion drive knob SVG");
        }
        
        auto toneKnobFile = basePath.getChildFile("Knob (large) (distortion - tone).svg");
        if (toneKnobFile.existsAsFile() && distortToneKnobSVG == nullptr)
        {
            distortToneKnobSVG = juce::Drawable::createFromSVGFile(toneKnobFile);
            if (distortToneKnobSVG != nullptr)
                DBG("Loaded distortion tone knob SVG");
        }
        
        // If we have both, we're done
        if (distortDriveKnobSVG != nullptr && distortToneKnobSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadDistortControlSVGs()
{
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load distortion control SVGs with correct file name
        auto onSwitchFile = basePath.getChildFile("DIST SWITCH ON.svg");
        if (onSwitchFile.existsAsFile() && distortOnSwitchSVG == nullptr)
        {
            distortOnSwitchSVG = juce::Drawable::createFromSVGFile(onSwitchFile);
            if (distortOnSwitchSVG != nullptr)
                DBG("Loaded distortion on switch SVG");
        }
        
        auto prePostSwitchFile = basePath.getChildFile("DIST SWITCH ON.svg");  // Reuse same switch SVG
        if (prePostSwitchFile.existsAsFile() && distortPrePostSwitchSVG == nullptr)
        {
            distortPrePostSwitchSVG = juce::Drawable::createFromSVGFile(prePostSwitchFile);
            if (distortPrePostSwitchSVG != nullptr)
                DBG("Loaded distortion pre/post switch SVG");
        }
        
        auto typeSelectorFile = basePath.getChildFile("Distortion or noise type selector.svg");
        if (typeSelectorFile.existsAsFile() && distortTypeSelectorSVG == nullptr)
        {
            distortTypeSelectorSVG = juce::Drawable::createFromSVGFile(typeSelectorFile);
            if (distortTypeSelectorSVG != nullptr)
                DBG("Loaded distortion type selector SVG");
        }
        
        // If we have all three, we're done
        if (distortOnSwitchSVG != nullptr && distortPrePostSwitchSVG != nullptr && distortTypeSelectorSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawDistortKnobs(juce::Graphics& g)
{
    // Draw DRIVE knob with rotation
    if (distortDriveKnobSVG != nullptr)
    {
        auto bounds = distortDriveSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        // Calculate rotation angle based on slider value (-150° to +150°)
        auto normalizedValue = (distortDriveSlider.getValue() - distortDriveSlider.getMinimum()) / 
                              (distortDriveSlider.getMaximum() - distortDriveSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        distortDriveKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw TONE knob with rotation  
    if (distortToneKnobSVG != nullptr)
    {
        auto bounds = distortToneSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        // Calculate rotation angle based on slider value (-150° to +150°)
        auto normalizedValue = (distortToneSlider.getValue() - distortToneSlider.getMinimum()) / 
                              (distortToneSlider.getMaximum() - distortToneSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        distortToneKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawDistortControls(juce::Graphics& g)
{
    // Draw ON switch - use different SVG based on toggle state
    if (distortOnSwitchSVG != nullptr)
    {
        auto bounds = distortOnButton.getBounds().toFloat();
        
        if (distortOnButton.getToggleState())
        {
            // Draw the "on" state SVG
            distortOnSwitchSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else if (switchOffSVG != nullptr)
        {
            // Draw the "off" state SVG (shared switch off asset)
            switchOffSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }
    
    // Draw Pre/Post switch - use different SVG based on toggle state
    if (distortPrePostSwitchSVG != nullptr)
    {
        auto bounds = distortPrePostButton.getBounds().toFloat();
        
        if (distortPrePostButton.getToggleState())
        {
            // Draw the "on" state SVG (POST position)
            distortPrePostSwitchSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else if (switchOffSVG != nullptr)
        {
            // Draw the "off" state SVG (PRE position)
            switchOffSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }
    
    // Draw TYPE selector
    if (distortTypeSelectorSVG != nullptr)
    {
        auto bounds = distortTypeSelector.getBounds().toFloat();
        distortTypeSelectorSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        
        // Draw the current selection text on top of the SVG
        auto selectedText = distortTypeSelector.getText();
        if (selectedText.isNotEmpty())
        {
            g.setColour(juce::Colours::black);
            g.setFont(customFont.withHeight(14.0f));
            
            // Position text centered within the selector
            auto textBounds = bounds.reduced(8, 0);
            g.drawText(selectedText, textBounds.toNearestInt(), juce::Justification::centred, true);
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::setupMagneticKnobs()
{
    // COMP knob
    magneticCompSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    magneticCompSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    magneticCompSlider.setRange(0.0, 1.0, 0.01);
    magneticCompSlider.setValue(0.5);
    magneticCompSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    magneticCompSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    magneticCompSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    magneticCompSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(magneticCompSlider);
    
    // SAT knob
    magneticSatSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    magneticSatSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    magneticSatSlider.setRange(0.0, 1.0, 0.01);
    magneticSatSlider.setValue(0.5);
    magneticSatSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    magneticSatSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    magneticSatSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    magneticSatSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(magneticSatSlider);
    
    // X TALK knob
    magneticXTalkSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    magneticXTalkSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    magneticXTalkSlider.setRange(0.0, 1.0, 0.01);
    magneticXTalkSlider.setValue(0.5);
    magneticXTalkSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    magneticXTalkSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    magneticXTalkSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    magneticXTalkSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(magneticXTalkSlider);
    
    // HEAD knob
    magneticHeadSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    magneticHeadSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    magneticHeadSlider.setRange(0.0, 1.0, 0.01);
    magneticHeadSlider.setValue(0.5);
    magneticHeadSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    magneticHeadSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    magneticHeadSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    magneticHeadSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(magneticHeadSlider);
    
    // WEAR knob
    magneticWearSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    magneticWearSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    magneticWearSlider.setRange(0.0, 1.0, 0.01);
    magneticWearSlider.setValue(0.5);
    magneticWearSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    magneticWearSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    magneticWearSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    magneticWearSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(magneticWearSlider);
    
    // Add repaint callbacks for visual feedback (like wobble module)
    magneticCompSlider.onValueChange = [this]() { repaint(); };
    magneticSatSlider.onValueChange = [this]() { repaint(); };
    magneticXTalkSlider.onValueChange = [this]() { repaint(); };
    magneticHeadSlider.onValueChange = [this]() { repaint(); };
    magneticWearSlider.onValueChange = [this]() { repaint(); };
    
    // Create parameter attachments
    magneticCompAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "magComp", magneticCompSlider);
    magneticSatAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "magSat", magneticSatSlider);
    magneticXTalkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "magCrosstalk", magneticXTalkSlider);
    magneticHeadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "magHeadBumpHz", magneticHeadSlider);
    magneticWearAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "magWear", magneticWearSlider);
}

void ReallyCheapTwentyAudioProcessorEditor::setupMagneticControls()
{
    // ON switch
    magneticOnButton.setButtonText("");
    magneticOnButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    magneticOnButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    magneticOnButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(magneticOnButton);
    
    // Add repaint callback for visual feedback (like wobble module)
    magneticOnButton.onStateChange = [this]() { repaint(); };
    
    // Create parameter attachment
    magneticOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "magOn", magneticOnButton);
}

void ReallyCheapTwentyAudioProcessorEditor::setupNoiseKnobs()
{
    // Level knob
    noiseLevelSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    noiseLevelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    noiseLevelSlider.setRange(0.0, 1.0, 0.01);
    noiseLevelSlider.setValue(0.5);
    noiseLevelSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    noiseLevelSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    noiseLevelSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(noiseLevelSlider);
    
    // Age knob
    noiseAgeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    noiseAgeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    noiseAgeSlider.setRange(0.0, 1.0, 0.01);
    noiseAgeSlider.setValue(0.5);
    noiseAgeSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    noiseAgeSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    noiseAgeSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(noiseAgeSlider);
    
    // Flutter knob
    noiseFlutterSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    noiseFlutterSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    noiseFlutterSlider.setRange(0.0, 1.0, 0.01);
    noiseFlutterSlider.setValue(0.5);
    noiseFlutterSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    noiseFlutterSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    noiseFlutterSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(noiseFlutterSlider);
    
    // Width knob
    noiseWidthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    noiseWidthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    noiseWidthSlider.setRange(0.0, 1.0, 0.01);
    noiseWidthSlider.setValue(0.5);
    noiseWidthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    noiseWidthSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    noiseWidthSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(noiseWidthSlider);
    
    // Add visual feedback callbacks before parameter attachments
    noiseLevelSlider.onValueChange = [this]() { repaint(); };
    noiseAgeSlider.onValueChange = [this]() { repaint(); };
    noiseFlutterSlider.onValueChange = [this]() { repaint(); };
    noiseWidthSlider.onValueChange = [this]() { repaint(); };
    
    // Create parameter attachments
    noiseLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "noiseLevel", noiseLevelSlider);
    noiseAgeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "noiseAge", noiseAgeSlider);
    noiseFlutterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "noiseFlutterGate", noiseFlutterSlider);
    noiseWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "noiseWidth", noiseWidthSlider);
}

void ReallyCheapTwentyAudioProcessorEditor::setupNoiseControls()
{
    // ON switch
    noiseOnButton.setButtonText("");
    noiseOnButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    noiseOnButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    noiseOnButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(noiseOnButton);
    
    // Pre/Post switch
    noisePrePostButton.setButtonText("");
    noisePrePostButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    noisePrePostButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    noisePrePostButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(noisePrePostButton);
    
    // Type selector
    noiseTypeSelector.addItem("Vinyl", 1);      // enum 0
    noiseTypeSelector.addItem("Tape", 2);       // enum 1
    noiseTypeSelector.addItem("Hum", 3);        // enum 2
    noiseTypeSelector.addItem("Fan", 4);        // enum 3
    noiseTypeSelector.addItem("Jazz Club", 5);  // enum 4 (Store PA removed)
    noiseTypeSelector.setSelectedId(1);
    noiseTypeSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    noiseTypeSelector.setColour(juce::ComboBox::textColourId, juce::Colours::transparentBlack);
    noiseTypeSelector.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    noiseTypeSelector.setColour(juce::ComboBox::arrowColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(noiseTypeSelector);
    
    // Type display label - positioned inside selector with black text
    noiseTypeLabel.setText("Vinyl", juce::dontSendNotification);
    noiseTypeLabel.setJustificationType(juce::Justification::centred);
    noiseTypeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    noiseTypeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    noiseTypeLabel.setFont(customFont.withHeight(14.0f));
    noiseTypeLabel.setInterceptsMouseClicks(false, false); // Allow clicks to pass through to selector
    addAndMakeVisible(noiseTypeLabel);
    
    // Add visual feedback callbacks before parameter attachments
    noiseOnButton.onStateChange = [this]() { repaint(); };
    noisePrePostButton.onStateChange = [this]() { repaint(); };
    noiseTypeSelector.onChange = [this]() { 
        noiseTypeLabel.setText(noiseTypeSelector.getText(), juce::dontSendNotification);
        repaint(); 
    };
    
    // Create parameter attachments
    noiseOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "noiseOn", noiseOnButton);
    noisePrePostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "noisePlacement", noisePrePostButton);
    noiseTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "noiseType", noiseTypeSelector);
}

void ReallyCheapTwentyAudioProcessorEditor::setupSpaceKnobs()
{
    // Mix knob
    spaceMixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    spaceMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    spaceMixSlider.setRange(0.0, 1.0, 0.01);
    spaceMixSlider.setValue(0.5);
    spaceMixSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    spaceMixSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    spaceMixSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(spaceMixSlider);
    
    // Time knob
    spaceTimeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    spaceTimeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    spaceTimeSlider.setRange(0.0, 1.0, 0.01);
    spaceTimeSlider.setValue(0.5);
    spaceTimeSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    spaceTimeSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    spaceTimeSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(spaceTimeSlider);
    
    // Tone knob
    spaceToneSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    spaceToneSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    spaceToneSlider.setRange(0.0, 1.0, 0.01);
    spaceToneSlider.setValue(0.5);
    spaceToneSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    spaceToneSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    spaceToneSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(spaceToneSlider);
    
    // Pre-delay knob
    spacePreDelaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    spacePreDelaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    spacePreDelaySlider.setRange(0.0, 1.0, 0.01);
    spacePreDelaySlider.setValue(0.5);
    spacePreDelaySlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    spacePreDelaySlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    spacePreDelaySlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(spacePreDelaySlider);
    
    // Cheapo knob
    spaceCheapoSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    spaceCheapoSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    spaceCheapoSlider.setRange(0.0, 1.0, 0.01);
    spaceCheapoSlider.setValue(0.5);
    spaceCheapoSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    spaceCheapoSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    spaceCheapoSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(spaceCheapoSlider);
    
    // Add visual feedback callbacks before parameter attachments
    spaceMixSlider.onValueChange = [this]() { repaint(); };
    spaceTimeSlider.onValueChange = [this]() { repaint(); };
    spaceToneSlider.onValueChange = [this]() { repaint(); };
    spacePreDelaySlider.onValueChange = [this]() { repaint(); };
    spaceCheapoSlider.onValueChange = [this]() { repaint(); };
    
    // Create parameter attachments
    spaceMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "spaceMix", spaceMixSlider);
    spaceTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "spaceTime", spaceTimeSlider);
    spaceToneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "spaceTone", spaceToneSlider);
    spacePreDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "spacePreDelayMs", spacePreDelaySlider);
    spaceCheapoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "spaceCheapo", spaceCheapoSlider);
}

void ReallyCheapTwentyAudioProcessorEditor::setupSpaceControls()
{
    // ON switch
    spaceOnButton.setButtonText("");
    spaceOnButton.setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    spaceOnButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    spaceOnButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(spaceOnButton);
    
    // Add visual feedback callback before parameter attachment
    spaceOnButton.onStateChange = [this]() { repaint(); };
    
    // Create parameter attachment
    spaceOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "spaceOn", spaceOnButton);
}

void ReallyCheapTwentyAudioProcessorEditor::loadMagneticKnobSVGs()
{
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load magnetic knob SVGs with correct file names
        auto compKnobFile = basePath.getChildFile("Knob Small (magnetic -comp).svg");
        if (compKnobFile.existsAsFile() && magneticCompKnobSVG == nullptr)
        {
            magneticCompKnobSVG = juce::Drawable::createFromSVGFile(compKnobFile);
            if (magneticCompKnobSVG != nullptr)
                DBG("Loaded magnetic comp knob SVG");
        }
        
        auto satKnobFile = basePath.getChildFile("Knob Small (magnetic - sat).svg");
        if (satKnobFile.existsAsFile() && magneticSatKnobSVG == nullptr)
        {
            magneticSatKnobSVG = juce::Drawable::createFromSVGFile(satKnobFile);
            if (magneticSatKnobSVG != nullptr)
                DBG("Loaded magnetic sat knob SVG");
        }
        
        auto xtalkKnobFile = basePath.getChildFile("Knob Small (magnetic - x talk).svg");
        if (xtalkKnobFile.existsAsFile() && magneticXTalkKnobSVG == nullptr)
        {
            magneticXTalkKnobSVG = juce::Drawable::createFromSVGFile(xtalkKnobFile);
            if (magneticXTalkKnobSVG != nullptr)
                DBG("Loaded magnetic xtalk knob SVG");
        }
        
        auto headKnobFile = basePath.getChildFile("Knob Small (magnetic - head).svg");
        if (headKnobFile.existsAsFile() && magneticHeadKnobSVG == nullptr)
        {
            magneticHeadKnobSVG = juce::Drawable::createFromSVGFile(headKnobFile);
            if (magneticHeadKnobSVG != nullptr)
                DBG("Loaded magnetic head knob SVG");
        }
        
        auto wearKnobFile = basePath.getChildFile("Knob Small (magnetic - wear).svg");
        if (wearKnobFile.existsAsFile() && magneticWearKnobSVG == nullptr)
        {
            magneticWearKnobSVG = juce::Drawable::createFromSVGFile(wearKnobFile);
            if (magneticWearKnobSVG != nullptr)
                DBG("Loaded magnetic wear knob SVG");
        }
        
        // If we have all knobs, we're done
        if (magneticCompKnobSVG != nullptr && magneticSatKnobSVG != nullptr && 
            magneticXTalkKnobSVG != nullptr && magneticHeadKnobSVG != nullptr && 
            magneticWearKnobSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadMagneticControlSVGs()
{
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load magnetic control SVGs with correct file name
        auto onSwitchFile = basePath.getChildFile("MAGNET SWITCH ON.svg");
        if (onSwitchFile.existsAsFile() && magneticOnSwitchSVG == nullptr)
        {
            magneticOnSwitchSVG = juce::Drawable::createFromSVGFile(onSwitchFile);
            if (magneticOnSwitchSVG != nullptr)
                DBG("Loaded magnetic on switch SVG");
        }
        
        // If we have the switch, we're done
        if (magneticOnSwitchSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadNoiseKnobSVGs()
{
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load noise knob SVGs with correct file names
        auto levelKnobFile = basePath.getChildFile("Knob Small (noise - level).svg");
        if (levelKnobFile.existsAsFile() && noiseLevelKnobSVG == nullptr)
        {
            noiseLevelKnobSVG = juce::Drawable::createFromSVGFile(levelKnobFile);
            if (noiseLevelKnobSVG != nullptr)
                DBG("Loaded atmosphere level knob SVG (formerly noise)");
        }
        
        auto ageKnobFile = basePath.getChildFile("Knob Small (noise - age).svg");
        if (ageKnobFile.existsAsFile() && noiseAgeKnobSVG == nullptr)
        {
            noiseAgeKnobSVG = juce::Drawable::createFromSVGFile(ageKnobFile);
            if (noiseAgeKnobSVG != nullptr)
                DBG("Loaded atmosphere age knob SVG (formerly noise)");
        }
        
        auto flutterKnobFile = basePath.getChildFile("Knob Small (noise - flutter).svg");
        if (flutterKnobFile.existsAsFile() && noiseFlutterKnobSVG == nullptr)
        {
            noiseFlutterKnobSVG = juce::Drawable::createFromSVGFile(flutterKnobFile);
            if (noiseFlutterKnobSVG != nullptr)
                DBG("Loaded atmosphere flutter knob SVG (formerly noise)");
        }
        
        auto widthKnobFile = basePath.getChildFile("Knob Small (noise - width).svg");
        if (widthKnobFile.existsAsFile() && noiseWidthKnobSVG == nullptr)
        {
            noiseWidthKnobSVG = juce::Drawable::createFromSVGFile(widthKnobFile);
            if (noiseWidthKnobSVG != nullptr)
                DBG("Loaded atmosphere width knob SVG (formerly noise)");
        }
        
        // If we have all knobs, we're done
        if (noiseLevelKnobSVG != nullptr && noiseAgeKnobSVG != nullptr && 
            noiseFlutterKnobSVG != nullptr && noiseWidthKnobSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadNoiseControlSVGs()
{
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load noise control SVGs with correct file names
        auto onSwitchFile = basePath.getChildFile("NOISE SWITCH ON.svg");
        if (onSwitchFile.existsAsFile() && noiseOnSwitchSVG == nullptr)
        {
            noiseOnSwitchSVG = juce::Drawable::createFromSVGFile(onSwitchFile);
            if (noiseOnSwitchSVG != nullptr)
                DBG("Loaded atmosphere ON switch SVG (formerly noise)");
        }
        
        auto prePostSwitchFile = basePath.getChildFile("NOISE SWITCH PRE/POST.svg");
        if (prePostSwitchFile.existsAsFile() && noisePrePostSwitchSVG == nullptr)
        {
            noisePrePostSwitchSVG = juce::Drawable::createFromSVGFile(prePostSwitchFile);
            if (noisePrePostSwitchSVG != nullptr)
                DBG("Loaded atmosphere Pre/Post switch SVG (formerly noise)");
        }
        
        auto typeSelectorFile = basePath.getChildFile("Distortion or noise type selector.svg");
        if (typeSelectorFile.existsAsFile() && noiseTypeSelectorSVG == nullptr)
        {
            noiseTypeSelectorSVG = juce::Drawable::createFromSVGFile(typeSelectorFile);
            if (noiseTypeSelectorSVG != nullptr)
                DBG("Loaded atmosphere type selector SVG (formerly noise)");
        }
        
        // If we have all controls, we're done
        if (noiseOnSwitchSVG != nullptr && noisePrePostSwitchSVG != nullptr && noiseTypeSelectorSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadSpaceKnobSVGs()
{
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load space knob SVGs with correct file names
        auto mixKnobFile = basePath.getChildFile("Knob Small (space- mix).svg");
        if (mixKnobFile.existsAsFile() && spaceMixKnobSVG == nullptr)
        {
            spaceMixKnobSVG = juce::Drawable::createFromSVGFile(mixKnobFile);
            if (spaceMixKnobSVG != nullptr)
                DBG("Loaded verb mix knob SVG (formerly space)");
        }
        
        auto timeKnobFile = basePath.getChildFile("Knob Small (space - time).svg");
        if (timeKnobFile.existsAsFile() && spaceTimeKnobSVG == nullptr)
        {
            spaceTimeKnobSVG = juce::Drawable::createFromSVGFile(timeKnobFile);
            if (spaceTimeKnobSVG != nullptr)
                DBG("Loaded verb time knob SVG (formerly space)");
        }
        
        auto toneKnobFile = basePath.getChildFile("Knob Small (space - tone).svg");
        if (toneKnobFile.existsAsFile() && spaceToneKnobSVG == nullptr)
        {
            spaceToneKnobSVG = juce::Drawable::createFromSVGFile(toneKnobFile);
            if (spaceToneKnobSVG != nullptr)
                DBG("Loaded verb tone knob SVG (formerly space)");
        }
        
        auto preDelayKnobFile = basePath.getChildFile("Knob Small (space - pre-delay).svg");
        if (preDelayKnobFile.existsAsFile() && spacePreDelayKnobSVG == nullptr)
        {
            spacePreDelayKnobSVG = juce::Drawable::createFromSVGFile(preDelayKnobFile);
            if (spacePreDelayKnobSVG != nullptr)
                DBG("Loaded verb pre-delay knob SVG (formerly space)");
        }
        
        auto cheapoKnobFile = basePath.getChildFile("Knob Small (space -cheapo).svg");
        if (cheapoKnobFile.existsAsFile() && spaceCheapoKnobSVG == nullptr)
        {
            spaceCheapoKnobSVG = juce::Drawable::createFromSVGFile(cheapoKnobFile);
            if (spaceCheapoKnobSVG != nullptr)
                DBG("Loaded verb cheapo knob SVG (formerly space)");
        }
        
        // If we have all knobs, we're done
        if (spaceMixKnobSVG != nullptr && spaceTimeKnobSVG != nullptr && 
            spaceToneKnobSVG != nullptr && spacePreDelayKnobSVG != nullptr && 
            spaceCheapoKnobSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::loadSpaceControlSVGs()
{
    juce::Array<juce::File> possiblePaths;
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\assets"));
    possiblePaths.add(juce::File("C:\\Users\\Owner\\Desktop\\DEV\\ReallyCheap-Twenty\\ReallyCheap-Twenty SVG assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("assets"));
    possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("ReallyCheap-Twenty SVG assets"));
    
    for (auto& basePath : possiblePaths)
    {
        // Try to load space control SVGs with correct file name
        auto onSwitchFile = basePath.getChildFile("SPACE SWITCH ON.svg");
        if (onSwitchFile.existsAsFile() && spaceOnSwitchSVG == nullptr)
        {
            spaceOnSwitchSVG = juce::Drawable::createFromSVGFile(onSwitchFile);
            if (spaceOnSwitchSVG != nullptr)
                DBG("Loaded verb ON switch SVG (formerly space)");
        }
        
        // If we have the switch, we're done
        if (spaceOnSwitchSVG != nullptr)
        {
            break;
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawMagneticKnobs(juce::Graphics& g)
{
    // Draw COMP knob with rotation
    if (magneticCompKnobSVG != nullptr)
    {
        auto bounds = magneticCompSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        // Calculate rotation angle based on slider value (-150° to +150°)
        auto normalizedValue = (magneticCompSlider.getValue() - magneticCompSlider.getMinimum()) / 
                              (magneticCompSlider.getMaximum() - magneticCompSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        magneticCompKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw SAT knob with rotation
    if (magneticSatKnobSVG != nullptr)
    {
        auto bounds = magneticSatSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (magneticSatSlider.getValue() - magneticSatSlider.getMinimum()) / 
                              (magneticSatSlider.getMaximum() - magneticSatSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        magneticSatKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw X TALK knob with rotation
    if (magneticXTalkKnobSVG != nullptr)
    {
        auto bounds = magneticXTalkSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (magneticXTalkSlider.getValue() - magneticXTalkSlider.getMinimum()) / 
                              (magneticXTalkSlider.getMaximum() - magneticXTalkSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        magneticXTalkKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw HEAD knob with rotation
    if (magneticHeadKnobSVG != nullptr)
    {
        auto bounds = magneticHeadSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (magneticHeadSlider.getValue() - magneticHeadSlider.getMinimum()) / 
                              (magneticHeadSlider.getMaximum() - magneticHeadSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        magneticHeadKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw WEAR knob with rotation
    if (magneticWearKnobSVG != nullptr)
    {
        auto bounds = magneticWearSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (magneticWearSlider.getValue() - magneticWearSlider.getMinimum()) / 
                              (magneticWearSlider.getMaximum() - magneticWearSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        magneticWearKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawMagneticControls(juce::Graphics& g)
{
    // Draw ON switch - use different SVG based on toggle state
    if (magneticOnSwitchSVG != nullptr)
    {
        auto bounds = magneticOnButton.getBounds().toFloat();
        
        if (magneticOnButton.getToggleState())
        {
            // Draw the "on" state SVG
            magneticOnSwitchSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else if (switchOffSVG != nullptr)
        {
            // Draw the "off" state SVG (shared switch off asset)
            switchOffSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawNoiseKnobs(juce::Graphics& g)
{
    // Draw Level knob with rotation
    if (noiseLevelKnobSVG != nullptr)
    {
        auto bounds = noiseLevelSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        // Calculate rotation angle based on slider value (-150° to +150°)
        auto normalizedValue = (noiseLevelSlider.getValue() - noiseLevelSlider.getMinimum()) / 
                              (noiseLevelSlider.getMaximum() - noiseLevelSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        noiseLevelKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw Age knob with rotation
    if (noiseAgeKnobSVG != nullptr)
    {
        auto bounds = noiseAgeSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (noiseAgeSlider.getValue() - noiseAgeSlider.getMinimum()) / 
                              (noiseAgeSlider.getMaximum() - noiseAgeSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        noiseAgeKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw Flutter knob with rotation
    if (noiseFlutterKnobSVG != nullptr)
    {
        auto bounds = noiseFlutterSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (noiseFlutterSlider.getValue() - noiseFlutterSlider.getMinimum()) / 
                              (noiseFlutterSlider.getMaximum() - noiseFlutterSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        noiseFlutterKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw Width knob with rotation
    if (noiseWidthKnobSVG != nullptr)
    {
        auto bounds = noiseWidthSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (noiseWidthSlider.getValue() - noiseWidthSlider.getMinimum()) / 
                              (noiseWidthSlider.getMaximum() - noiseWidthSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        noiseWidthKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawNoiseControls(juce::Graphics& g)
{
    // Draw ON switch - use different SVG based on toggle state
    if (noiseOnSwitchSVG != nullptr)
    {
        auto bounds = noiseOnButton.getBounds().toFloat();
        
        if (noiseOnButton.getToggleState())
        {
            // Draw the "on" state SVG
            noiseOnSwitchSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else if (switchOffSVG != nullptr)
        {
            // Draw the "off" state SVG (shared switch off asset)
            switchOffSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }
    
    // Draw Pre/Post switch - use different SVG based on toggle state
    if (noisePrePostSwitchSVG != nullptr)
    {
        auto bounds = noisePrePostButton.getBounds().toFloat();
        
        if (noisePrePostButton.getToggleState())
        {
            // Draw the "on" state SVG (Post)
            noisePrePostSwitchSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else if (switchOffSVG != nullptr)
        {
            // Draw the "off" state SVG (Pre)
            switchOffSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }
    
    // Draw Type selector
    if (noiseTypeSelectorSVG != nullptr)
    {
        auto bounds = noiseTypeSelector.getBounds().toFloat();
        noiseTypeSelectorSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawSpaceKnobs(juce::Graphics& g)
{
    // Draw Mix knob with rotation
    if (spaceMixKnobSVG != nullptr)
    {
        auto bounds = spaceMixSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        // Calculate rotation angle based on slider value (-150° to +150°)
        auto normalizedValue = (spaceMixSlider.getValue() - spaceMixSlider.getMinimum()) / 
                              (spaceMixSlider.getMaximum() - spaceMixSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        spaceMixKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw Time knob with rotation
    if (spaceTimeKnobSVG != nullptr)
    {
        auto bounds = spaceTimeSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (spaceTimeSlider.getValue() - spaceTimeSlider.getMinimum()) / 
                              (spaceTimeSlider.getMaximum() - spaceTimeSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        spaceTimeKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw Tone knob with rotation
    if (spaceToneKnobSVG != nullptr)
    {
        auto bounds = spaceToneSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (spaceToneSlider.getValue() - spaceToneSlider.getMinimum()) / 
                              (spaceToneSlider.getMaximum() - spaceToneSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        spaceToneKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw Pre-delay knob with rotation
    if (spacePreDelayKnobSVG != nullptr)
    {
        auto bounds = spacePreDelaySlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (spacePreDelaySlider.getValue() - spacePreDelaySlider.getMinimum()) / 
                              (spacePreDelaySlider.getMaximum() - spacePreDelaySlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        spacePreDelayKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
    
    // Draw Cheapo knob with rotation
    if (spaceCheapoKnobSVG != nullptr)
    {
        auto bounds = spaceCheapoSlider.getBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        auto normalizedValue = (spaceCheapoSlider.getValue() - spaceCheapoSlider.getMinimum()) / 
                              (spaceCheapoSlider.getMaximum() - spaceCheapoSlider.getMinimum());
        auto rotationAngle = -150.0f + (normalizedValue * 300.0f);
        
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationAngle), centerX, centerY));
        spaceCheapoKnobSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }
}

void ReallyCheapTwentyAudioProcessorEditor::drawSpaceControls(juce::Graphics& g)
{
    // Draw ON switch - use different SVG based on toggle state
    if (spaceOnSwitchSVG != nullptr)
    {
        auto bounds = spaceOnButton.getBounds().toFloat();
        
        if (spaceOnButton.getToggleState())
        {
            // Draw the "on" state SVG
            spaceOnSwitchSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else if (switchOffSVG != nullptr)
        {
            // Draw the "off" state SVG (shared switch off asset)
            switchOffSVG->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }
}

void ReallyCheapTwentyAudioProcessorEditor::updateUIScale()
{
    // This gets called automatically by resized(), no action needed
}

void ReallyCheapTwentyAudioProcessorEditor::updateUIScale(float newScale)
{
    // Calculate new size based on scale
    int baseWidth = 1074;  // Base width (50% of Figma)
    int baseHeight = 598;  // Base height (50% of Figma) 
    
    int newWidth = static_cast<int>(baseWidth * newScale);
    int newHeight = static_cast<int>(baseHeight * newScale);
    
    // Set the new size (this will trigger resized() which updates positions)
    setSize(newWidth, newHeight);
}