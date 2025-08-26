cmake_minimum_required(VERSION 3.15)

project(ReallyCheap-Twenty VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()


add_subdirectory(JUCE)

juce_add_plugin(ReallyCheap-Twenty
    COMPANY_NAME "ReallyCheap Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE RcCh
    PLUGIN_CODE RcT1
    FORMATS VST3 AU
    PRODUCT_NAME "ReallyCheap-Twenty"
    VST3_CATEGORIES "Fx" "Distortion"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

# Generate binary data from assets
juce_add_binary_data(NoiseAssets
    SOURCES
        "assets/noise/vinyl.mp3"
        "assets/noise/tape.mp3"
        "assets/noise/jazz club.mp3"
)

# Generate binary data for UI assets
juce_add_binary_data(UIAssets
    SOURCES
        "assets/background.png"
        "assets/Title_Card_Cleaner.png"
        "assets/knobs/golden_knob.png"
        "assets/knobs/golden_knob_filmstrip.png"
        "assets/knobs/knob_filmstrip.png"
        "assets/knobs/large_knob.png"
        "assets/knobs/large_knob_filmstrip.png"
        "assets/knobs/mockup_knob.png"
        "assets/knobs/single_knob.png"
        "assets/knobs/small_knob.png"
        "assets/knobs/small_knob_filmstrip.png"
)

juce_generate_juce_header(ReallyCheap-Twenty)

target_sources(ReallyCheap-Twenty
    PRIVATE
        Source/core/PluginProcessor.cpp
        Source/core/PluginEditor.cpp
        Source/core/Params.cpp
        Source/core/Presets.cpp
        Source/core/MacroController.cpp
        Source/dsp/Distort.cpp
        Source/dsp/Wobble.cpp
        Source/dsp/Digital.cpp
        Source/ui/LookAndFeel.cpp
        Source/ui/ModulePanels/DistortPanel.cpp
        Source/ui/ModulePanels/WobblePanel.cpp
        Source/ui/ModulePanels/DigitalPanel.cpp
        Source/ui/ModulePanels/MagneticPanel.cpp
        Source/ui/ModulePanels/NoisePanel.cpp
        Source/ui/ModulePanels/SpacePanel.cpp
        Source/dsp/Magnetic.cpp
        Source/dsp/Noise.cpp
        Source/dsp/noise/NoiseAssetManager.cpp
        Source/dsp/Space.cpp
        Source/dsp/space/SpaceIRManager.cpp
)

target_compile_definitions(ReallyCheap-Twenty
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_MODAL_LOOPS_PERMITTED=0
        JUCE_COREAUDIO_ALLOW_ALLOCATION_IN_REALTIME=0
)

target_link_libraries(ReallyCheap-Twenty
    PRIVATE
        NoiseAssets
        UIAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Add tests subdirectory
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()