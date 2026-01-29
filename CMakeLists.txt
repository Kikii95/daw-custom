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
        # Main
        Source/Main.cpp
        Source/MainComponent.cpp

        # Audio Core
        Source/Audio/AudioEngine.cpp
        Source/Audio/TransportController.cpp
        Source/Audio/AudioClip.cpp
        Source/Audio/AudioTrack.cpp
        Source/Audio/AudioMixer.cpp

        # Audio FileIO
        Source/Audio/FileIO/AudioFileLoader.cpp
        Source/Audio/FileIO/AudioFileExporter.cpp

        # Audio DSP
        Source/Audio/DSP/EffectChain.cpp
        Source/Audio/DSP/Effects/GainEffect.cpp
        Source/Audio/DSP/Effects/ParametricEQ.cpp
        Source/Audio/DSP/Effects/CompressorEffect.cpp
        Source/Audio/DSP/Effects/ReverbEffect.cpp
        Source/Audio/DSP/Effects/DelayEffect.cpp

        # Audio DSP - Synthesis
        Source/Audio/DSP/Synthesis/OscillatorEffect.cpp
        Source/Audio/DSP/Synthesis/EnvelopeEffect.cpp
        Source/Audio/DSP/Synthesis/KickDesigner.cpp
        Source/Audio/DSP/Synthesis/SimpleFilter.cpp
        Source/Audio/DSP/Synthesis/BasicSynth.cpp

        # Audio Plugins
        Source/Audio/Plugins/PluginManager.cpp
        Source/Audio/Plugins/VST3EffectSlot.cpp

        # UI - Main
        Source/UI/MainLayout.cpp

        # UI - Transport
        Source/UI/Transport/TransportBar.cpp

        # UI - Waveform
        Source/UI/Waveform/WaveformCache.cpp
        Source/UI/Waveform/WaveformDisplay.cpp

        # UI - Timeline
        Source/UI/Timeline/TimelineRuler.cpp
        Source/UI/Timeline/ClipComponent.cpp
        Source/UI/Timeline/TrackLane.cpp
        Source/UI/Timeline/TimelinePanel.cpp

        # UI - Mixer
        Source/UI/Mixer/MeterComponent.cpp
        Source/UI/Mixer/ChannelStrip.cpp
        Source/UI/Mixer/MixerPanel.cpp

        # UI - Effects
        Source/UI/Effects/EffectSlotComponent.cpp
        Source/UI/Effects/EffectRackPanel.cpp

        # UI - Plugins
        Source/UI/Plugins/PluginEditorWindow.cpp
        Source/UI/Plugins/GenericPluginEditor.cpp
        Source/UI/Plugins/PluginBrowser.cpp

        # UI - Assets
        Source/UI/Assets/AssetBrowser.cpp

        # UI - Theme
        Source/UI/Theme/ModernLookAndFeel.cpp
        Source/UI/Theme/DrawingHelpers.cpp

        # UI - Layout
        Source/UI/Layout/FlexPanel.cpp

        # UI - Dialogs
        Source/UI/Dialogs/FirstRunDialog.cpp
        Source/UI/Dialogs/ModalOverlay.cpp

        # Utils
        Source/Utils/Tweens.cpp
)

# Include directories for our headers
target_include_directories(DAWCustom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
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

# =============================================================================
# Tests (optional)
# =============================================================================
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()

    juce_add_console_app(DAWCustomTests
        PRODUCT_NAME "DAW Custom Tests"
    )

    target_sources(DAWCustomTests
        PRIVATE
            Source/Tests/TestRunner.cpp
    )

    target_include_directories(DAWCustomTests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Source
    )

    target_compile_definitions(DAWCustomTests
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
    )

    target_link_libraries(DAWCustomTests
        PRIVATE
            juce::juce_audio_basics
            juce::juce_core
        PUBLIC
            juce::juce_recommended_config_flags
    )

    add_test(NAME UnitTests COMMAND DAWCustomTests)
endif()
