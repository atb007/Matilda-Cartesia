cmake_minimum_required(VERSION 3.22)

project(GridWalker VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer local JUCE clone: git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(GridWalker
    COMPANY_NAME "IdeasLab"
    PLUGIN_MANUFACTURER_CODE Idlb
    PLUGIN_CODE GrWk
    FORMATS Standalone VST3 AU
    PRODUCT_NAME "GridWalker"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

juce_generate_juce_header(GridWalker)

target_sources(GridWalker
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/GridEngine.cpp
        Source/GridComponent.cpp
        Source/GridWalkerLookAndFeel.cpp
)

target_compile_definitions(GridWalker
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(GridWalker
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
