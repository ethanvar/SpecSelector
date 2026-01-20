cmake_minimum_required(VERSION 3.22)

project(SpecSelector VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(JUCE)

juce_add_plugin(SpecSelector
    VERSION 1.0.0
    BUNDLE_ID "ca.ethanvarghese.SpecSelector"
    COMPANY_NAME "ethanvarghese"
    IS_SYNTH false
    NEEDS_MIDI_INPUT false
    NEEDS_MIDI_OUTPUT false
    IS_MIDI_EFFECT true
    EDITOR_WANTS_KEYBOARD_FOCUS true
    COPY_PLUGIN_AFTER_BUILD true
    FORMATS Standalone VST3
    PLUGIN_MANUFACTURER_CODE "PLAI"
    PLUGIN_CODE "ETHV"
    PRODUCT_NAME "Spec Selector"
)

juce_generate_juce_header(SpecSelector)

target_sources(SpecSelector
    PRIVATE
        plugin/PluginProcessor.cpp
        plugin/PluginProcessor.h
        plugin/PluginEditor.cpp
        plugin/PluginEditor.h
)

target_compile_definitions(SpecSelector
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SpecSelector PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_formats
)
