cmake_minimum_required(VERSION 3.22)

project(DontDropTheHammer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent to automatically acquire JUCE if not already available locally
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(JUCE)

# Define the custom audio plugin target
juce_add_plugin(DontDropTheHammer
    COMPANY_NAME "THE EDGE OF FEAR"
    BUNDLE_ID com.theedgeoffear.dontdropthehammer
    PLUGIN_MANUFACTURER_CODE TEOF
    PLUGIN_CODE Ddth
    FORMATS VST3 Standalone
    PRODUCT_NAME "DONT DROP THE HAMMER"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Source files
target_sources(DontDropTheHammer
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/TuningModel.h
        Source/PitchShifterEngine.h
        Source/PitchDetector.h
        Source/StrobeTunerComponent.h
        Source/ExpressionPedalComponent.h
        Source/BrutalLookAndFeel.h
        Source/MidiManager.h
        Source/MidiMappingModal.h
        Source/PresetManager.h
)

# Binary assets
juce_add_binary_data(DDTH_Assets
    SOURCES
        Images/DROP_THE_HAMMER_BG.jpg
        Images/Viking_Coffin_Lid.png
)

# Link essential JUCE modules
target_link_libraries(DontDropTheHammer
    PRIVATE
        DDTH_Assets
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

# Global ASIO SDK support for all Standalone apps
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/asiosdk/common")

# Compile definitions
target_compile_definitions(DontDropTheHammer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_ASIO=1
)

# HARBINGER OF DEATH Plugin by THE EDGE OF FEAR
add_subdirectory("HARBINGER OF DEATH")

# THE BLEED Plugin by THE EDGE OF FEAR
add_subdirectory("THE BLEED")

# THE VOID Vocal Processor Plugin by THE EDGE OF FEAR
add_subdirectory("THE VOID")

# THE GOAT HM-2 Cult Chainsaw Plugin by THE EDGE OF FEAR
add_subdirectory("THE GOAT")

# FUZZY MONKEYS Dual High-Gain Doom Fuzz Plugin by THE EDGE OF FEAR
add_subdirectory("FUZZY MONKEYS")


