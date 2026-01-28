cmake_minimum_required(VERSION 3.22)

project(DAWCustom VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch JUCE
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

# Main application
juce_add_gui_app(DAWCustom
    PRODUCT_NAME "DAW Custom"
    COMPANY_NAME "Personal"
    BUNDLE_ID "com.personal.dawcustom"
)

# Sources
target_sources(DAWCustom
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
)

# JUCE modules
target_compile_definitions(DAWCustom
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(DAWCustom
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
