cmake_minimum_required(VERSION 3.22)

project(Matilda VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../gridwalker/JUCE/CMakeLists.txt")
    add_subdirectory(../../gridwalker/JUCE JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(Matilda
    COMPANY_NAME "IdeasLab"
    PLUGIN_MANUFACTURER_CODE Idlb
    PLUGIN_CODE Mtld
    FORMATS Standalone VST3 AU
    PRODUCT_NAME "Matilda"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

juce_generate_juce_header(Matilda)

juce_add_binary_data(MatildaAssets
    SOURCES
        # VCV Cartesia panel (source of visual truth)
        ../assets/ui/cartesia/Cartesia-dark.svg
        ../assets/ui/cartesia/DejaVuSansMono.ttf
        ../assets/fonts/Asimovian/Asimovian-Regular.ttf
        ../assets/fonts/Kode_Mono/static/KodeMono-Bold.ttf
        ../assets/fonts/Supermercado_One/SupermercadoOne-Regular.ttf
        ../assets/fonts/Jacquard_24/Jacquard24-Regular.ttf
        ../../cartesia-vst-ui/public/assets/movement-filigree-top.svg
        ../../cartesia-vst-ui/public/assets/movement-filigree-bottom.svg
        ../../cartesia-vst-ui/public/assets/movement-bgtexture.svg
        ../../cartesia-vst-ui/public/assets/movement-bgtexture@2xpng.png
        # Figma chrome overlays (portrait, crystal bar buttons)
        ../assets/ui/figma/crystal-gold.png
        ../assets/ui/figma/crystal-amber.png
        ../assets/ui/figma/play-button.png
        ../assets/ui/figma/gem-knob-glow.png
        # Main grid knob (Figma 4921:102056) — shared ring/sphere + per-layer gloss
        ../assets/ui/knob/orange/OuterRing2x.png
        ../assets/ui/knob/orange/DarkSpehere2x.png
        ../assets/ui/knob/orange/ColoredGloss-ON-2x.png
        ../assets/ui/knob/orange/ColoredGloss-OFF-2x.png
        ../assets/ui/knob/red/ColoredGloss-ON-2x.png
        ../assets/ui/knob/red/ColoredGloss-OFF-2x.png
        ../assets/ui/knob/green/ColoredGloss-ON-2x.png
        ../assets/ui/knob/green/ColoredGloss-OFF-2x.png
        ../assets/ui/knob/blue/ColoredGloss-ON-2x.png
        ../assets/ui/knob/blue/ColoredGloss-OFF-2x.png
        # React shell / hero (cartesia-vst-ui public assets)
        ../../cartesia-vst-ui/public/assets/shell-frame-vines-only@2x.png
        ../../cartesia-vst-ui/public/assets/shell-glass-bedding.png
        ../../cartesia-vst-ui/public/assets/shell-glass-radial.svg
        ../../cartesia-vst-ui/public/assets/hero-bg-m8b.png
        ../../cartesia-vst-ui/public/assets/collapse-toggle-expanded@2x.png
        ../../cartesia-vst-ui/public/assets/collapse-toggle-collapsed@2x.png
        ../../cartesia-vst-ui/public/assets/matilda-portrait-masked.png
        ../../cartesia-vst-ui/public/assets/matilda-mask-alpha.png
        ../../cartesia-vst-ui/public/assets/transport-play-gem.png
        ../../cartesia-vst-ui/public/assets/transport-play-frame.png
        ../../cartesia-vst-ui/public/assets/transport-glass-bg.png
        ../../cartesia-vst-ui/public/assets/transport-play-icon.png
        ../../cartesia-vst-ui/public/assets/transport-stop-icon.png
        ../../cartesia-vst-ui/public/assets/transport-title-filigree.svg
        ../../cartesia-vst-ui/public/assets/transport-title-rule.svg
        ../../cartesia-vst-ui/public/assets/transport-ornament-clock-left.svg
        ../../cartesia-vst-ui/public/assets/transport-ornament-clock-right.svg
        ../../cartesia-vst-ui/public/assets/minigrid-frame.png
        ../../cartesia-vst-ui/public/assets/minigrid-inactive.png
        ../../cartesia-vst-ui/public/assets/minigrid-orange-on.png
        ../../cartesia-vst-ui/public/assets/minigrid-orange-off.png
        ../../cartesia-vst-ui/public/assets/minigrid-red-on.png
        ../../cartesia-vst-ui/public/assets/minigrid-red-off.png
        ../../cartesia-vst-ui/public/assets/minigrid-green-on.png
        ../../cartesia-vst-ui/public/assets/minigrid-green-off.png
        ../../cartesia-vst-ui/public/assets/minigrid-blue-on.png
        ../../cartesia-vst-ui/public/assets/minigrid-blue-off.png
        ../../cartesia-vst-ui/public/assets/scale-gem-chromatic.png
        ../../cartesia-vst-ui/public/assets/scale-gem-major.png
        ../../cartesia-vst-ui/public/assets/scale-gem-minor.png
        ../../cartesia-vst-ui/public/assets/scale-gem-dorian.png
        ../../cartesia-vst-ui/public/assets/scale-gem-phrygian.png
        ../../cartesia-vst-ui/public/assets/scale-gem-lydian.png
        ../../cartesia-vst-ui/public/assets/scale-gem-mixolydian.png
        ../../cartesia-vst-ui/public/assets/scale-gem-locrian.png
        ../../cartesia-vst-ui/public/assets/scale-gem-harmonic_minor.png
        ../../cartesia-vst-ui/public/assets/scale-gem-melodic_minor.png
        ../../cartesia-vst-ui/public/assets/scale-gem-pentatonic.png
        ../../cartesia-vst-ui/public/assets/scale-gem-pentatonic_minor.png
        ../../cartesia-vst-ui/public/assets/scale-gem-blues.png
        ../../cartesia-vst-ui/public/assets/scale-title-filigree-top.svg
        ../../cartesia-vst-ui/public/assets/scale-title-filigree-bottom.svg
        ../../cartesia-vst-ui/public/assets/scale-title-rule.svg
        ../../cartesia-vst-ui/public/assets/scale-minmax-ornament-left.svg
        ../../cartesia-vst-ui/public/assets/scale-minmax-ornament-right.svg
        ../../cartesia-vst-ui/public/assets/scale-connector-left.svg
        ../../cartesia-vst-ui/public/assets/scale-connector-right.svg
        ../../cartesia-vst-ui/public/assets/scale-arrow-left.svg
        ../../cartesia-vst-ui/public/assets/scale-arrow-right.svg
        # Default Patch v2 preset
        ../presets/default.layer1.json
)

target_sources(Matilda
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/MatildaLookAndFeel.cpp
        Source/MatildaFonts.cpp
        Source/MatildaImages.cpp
        Source/ShellGlassDrawing.cpp
        Source/MatildaBackground.cpp
        Source/Components/GemCell.cpp
        Source/Components/GemGrid.cpp
        Source/Components/LayerPathView.cpp
        Source/Components/LayerOverview.cpp
        Source/Components/LayerCrystalBar.cpp
        Source/Components/QuantisePanel.cpp
        Source/Components/GemSparksOverlay.cpp
        Source/Components/TransportBar.cpp
        Source/Components/MovementSelector.cpp
        Source/Components/StepPathStrip.cpp
        Source/Components/MatildaShellPanel.cpp
        Source/Components/ShellChrome.cpp
        Source/Components/HeroCanvas.cpp
        Source/Components/NativePluginFrame.cpp
        Source/Components/UiResizeGrip.cpp
        Source/Components/CollapseToggle.cpp
        Source/Engine/SequencerEngine.cpp
        Source/Engine/PatchStore.cpp
)

target_compile_definitions(Matilda
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Matilda
    PRIVATE
        MatildaAssets
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
