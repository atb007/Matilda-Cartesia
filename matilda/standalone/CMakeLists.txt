cmake_minimum_required(VERSION 3.22)

# Matilda Standalone-only codebase (forked from plugin v1.0.2, before DAW-coupling regressions).
# Kept independent from matilda/plugin so the macOS standalone app stays known-good.
project(MatildaStandalone VERSION 1.0.11 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Native Rive hero — Metal (macOS), D3D11 (Windows), CG CPU fallback (macOS only).
if(APPLE)
    option(MATILDA_RIVE_HERO "Enable native Rive hero portrait (macOS)" ON)
    set(MATILDA_RIVE_BACKEND "metal" CACHE STRING "Rive renderer backend: metal (GPU) or cg (CPU fallback)")
    set_property(CACHE MATILDA_RIVE_BACKEND PROPERTY STRINGS metal cg)
elseif(WIN32)
    option(MATILDA_RIVE_HERO "Enable native Rive hero portrait (Windows)" ON)
    set(MATILDA_RIVE_BACKEND "d3d" CACHE STRING "Rive renderer backend: d3d (GPU)")
    set_property(CACHE MATILDA_RIVE_BACKEND PROPERTY STRINGS d3d)
else()
    set(MATILDA_RIVE_HERO OFF CACHE BOOL "" FORCE)
    set(MATILDA_RIVE_BACKEND "cg" CACHE STRING "" FORCE)
endif()

if(MATILDA_RIVE_HERO AND NOT (APPLE OR WIN32))
    message(FATAL_ERROR "MATILDA_RIVE_HERO is supported on macOS and Windows builds only.")
endif()

if(MATILDA_RIVE_HERO AND WIN32 AND NOT MATILDA_RIVE_BACKEND STREQUAL "d3d")
    message(FATAL_ERROR "Windows MATILDA_RIVE_HERO requires MATILDA_RIVE_BACKEND=d3d.")
endif()

if(MATILDA_RIVE_HERO AND APPLE AND MATILDA_RIVE_BACKEND STREQUAL "d3d")
    message(FATAL_ERROR "macOS MATILDA_RIVE_HERO does not support MATILDA_RIVE_BACKEND=d3d.")
endif()

set(MATILDA_RIVE_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/rive-runtime")
set(MATILDA_RIVE_LIB_DIR "${MATILDA_RIVE_RUNTIME_DIR}/matilda/out/release")

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
    FORMATS Standalone
    PRODUCT_NAME "Matilda"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/matilda-app-icon.png"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

juce_generate_juce_header(Matilda)

set(MATILDA_ASSET_SOURCES
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

if(MATILDA_RIVE_HERO)
    list(APPEND MATILDA_ASSET_SOURCES
        ../../cartesia-vst-ui/public/assets/matilda-cartesia-v2.riv
    )
endif()

juce_add_binary_data(MatildaAssets SOURCES ${MATILDA_ASSET_SOURCES})

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

if(MATILDA_RIVE_HERO)
    target_sources(Matilda PRIVATE Source/Rive/RiveHeroRenderer.cpp)
    if(MATILDA_RIVE_BACKEND STREQUAL "metal")
        target_sources(Matilda PRIVATE
            Source/Rive/RiveHeroBackendMetal.mm
            Source/Rive/RiveHeroMetalCore.mm
            Source/Rive/RiveHeroMetalView.mm
        )
        set_source_files_properties(
            Source/Rive/RiveHeroBackendMetal.mm
            Source/Rive/RiveHeroMetalCore.mm
            Source/Rive/RiveHeroMetalView.mm
            PROPERTIES COMPILE_FLAGS "-x objective-c++")
    elseif(MATILDA_RIVE_BACKEND STREQUAL "d3d")
        target_sources(Matilda PRIVATE
            Source/Rive/RiveHeroBackendD3D.cpp
            Source/Rive/RiveHeroD3DCore.cpp
        )
        target_compile_definitions(Matilda PRIVATE NOMINMAX)
    else()
        target_sources(Matilda PRIVATE Source/Rive/RiveHeroBackendCG.cpp)
    endif()
endif()

target_compile_definitions(Matilda
    PRIVATE
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

if(MATILDA_RIVE_HERO)
    target_compile_definitions(Matilda PRIVATE MATILDA_RIVE_HERO=1 JUCE_WEB_BROWSER=0)
    if(MATILDA_RIVE_BACKEND STREQUAL "metal")
        target_compile_definitions(Matilda PRIVATE MATILDA_RIVE_BACKEND_METAL=1 MATILDA_RIVE_BACKEND_GPU=1)
    elseif(MATILDA_RIVE_BACKEND STREQUAL "d3d")
        target_compile_definitions(Matilda PRIVATE MATILDA_RIVE_BACKEND_D3D=1 MATILDA_RIVE_BACKEND_GPU=1)
    endif()

    if(APPLE)
        set(_RIVE_LIB_PREFIX "lib")
        set(_RIVE_LIB_EXT ".a")
        set(_RIVE_BUILD_SCRIPT "matilda/standalone/scripts/build_rive_native.sh")
    else()
        set(_RIVE_LIB_PREFIX "")
        set(_RIVE_LIB_EXT ".lib")
        set(_RIVE_BUILD_SCRIPT "matilda/standalone/scripts/build_rive_native.ps1")
    endif()

    if(NOT EXISTS "${MATILDA_RIVE_LIB_DIR}/${_RIVE_LIB_PREFIX}rive${_RIVE_LIB_EXT}")
        message(FATAL_ERROR
            "MATILDA_RIVE_HERO requires prebuilt Rive libraries.\n"
            "Run: MATILDA_RIVE_BACKEND=${MATILDA_RIVE_BACKEND} ${_RIVE_BUILD_SCRIPT}")
    endif()

    set(MATILDA_RIVE_INCLUDES
        "${MATILDA_RIVE_RUNTIME_DIR}/include"
        "${MATILDA_RIVE_RUNTIME_DIR}/decoders/include"
    )
    if(MATILDA_RIVE_BACKEND STREQUAL "metal" OR MATILDA_RIVE_BACKEND STREQUAL "d3d")
        list(APPEND MATILDA_RIVE_INCLUDES "${MATILDA_RIVE_RUNTIME_DIR}/renderer/include")
        if(NOT EXISTS "${MATILDA_RIVE_LIB_DIR}/${_RIVE_LIB_PREFIX}rive_pls_renderer${_RIVE_LIB_EXT}")
            message(FATAL_ERROR
                "GPU backend requires rive_pls_renderer.\n"
                "Run: MATILDA_RIVE_BACKEND=${MATILDA_RIVE_BACKEND} ${_RIVE_BUILD_SCRIPT}")
        endif()
    else()
        list(APPEND MATILDA_RIVE_INCLUDES "${MATILDA_RIVE_RUNTIME_DIR}/cg_renderer/include")
        if(NOT EXISTS "${MATILDA_RIVE_LIB_DIR}/${_RIVE_LIB_PREFIX}rive_cg_renderer${_RIVE_LIB_EXT}")
            message(FATAL_ERROR
                "CG backend requires rive_cg_renderer.\n"
                "Run: MATILDA_RIVE_BACKEND=cg ${_RIVE_BUILD_SCRIPT}")
        endif()
    endif()
    target_include_directories(Matilda PRIVATE ${MATILDA_RIVE_INCLUDES})
    target_compile_definitions(Matilda PRIVATE WITH_RIVE_TEXT WITH_RIVE_LAYOUT WITH_RIVE_SCRIPTING)
else()
    target_compile_definitions(Matilda PRIVATE JUCE_WEB_BROWSER=0)
endif()

target_link_libraries(Matilda
    PRIVATE
        MatildaAssets
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(MATILDA_RIVE_HERO)
    if(APPLE)
        set(MATILDA_RIVE_LIBS
            "-Wl,-force_load,${MATILDA_RIVE_LIB_DIR}/librive_decoders.a"
            "-Wl,-force_load,${MATILDA_RIVE_LIB_DIR}/librive.a"
            "${MATILDA_RIVE_LIB_DIR}/librive_harfbuzz.a"
            "${MATILDA_RIVE_LIB_DIR}/librive_sheenbidi.a"
            "${MATILDA_RIVE_LIB_DIR}/librive_yoga.a"
            "${MATILDA_RIVE_LIB_DIR}/libminiaudio.a"
            "${MATILDA_RIVE_LIB_DIR}/libluau_vm.a"
            "${MATILDA_RIVE_LIB_DIR}/liblibpng.a"
            "${MATILDA_RIVE_LIB_DIR}/liblibjpeg.a"
            "${MATILDA_RIVE_LIB_DIR}/liblibwebp.a"
            "${MATILDA_RIVE_LIB_DIR}/libzlib.a"
            "-framework ApplicationServices"
            "-framework CoreFoundation"
        )
        if(MATILDA_RIVE_BACKEND STREQUAL "metal")
            list(PREPEND MATILDA_RIVE_LIBS "-Wl,-force_load,${MATILDA_RIVE_LIB_DIR}/librive_pls_renderer.a")
            list(APPEND MATILDA_RIVE_LIBS "-framework Metal" "-framework QuartzCore")
        else()
            list(PREPEND MATILDA_RIVE_LIBS "-Wl,-force_load,${MATILDA_RIVE_LIB_DIR}/librive_cg_renderer.a")
            list(APPEND MATILDA_RIVE_LIBS
                "-framework CoreGraphics" "-framework CoreText" "-framework ImageIO")
        endif()
    else()
        set(MATILDA_RIVE_LIBS
            "${MATILDA_RIVE_LIB_DIR}/rive_harfbuzz.lib"
            "${MATILDA_RIVE_LIB_DIR}/rive_sheenbidi.lib"
            "${MATILDA_RIVE_LIB_DIR}/rive_yoga.lib"
            "${MATILDA_RIVE_LIB_DIR}/miniaudio.lib"
            "${MATILDA_RIVE_LIB_DIR}/luau_vm.lib"
            "${MATILDA_RIVE_LIB_DIR}/libpng.lib"
            "${MATILDA_RIVE_LIB_DIR}/libjpeg.lib"
            "${MATILDA_RIVE_LIB_DIR}/libwebp.lib"
            "${MATILDA_RIVE_LIB_DIR}/zlib.lib"
            d3d11.lib
            d3d12.lib
            d3dcompiler.lib
            dxgi.lib
        )
        target_link_options(Matilda PUBLIC
            "/WHOLEARCHIVE:${MATILDA_RIVE_LIB_DIR}/rive_pls_renderer.lib"
            "/WHOLEARCHIVE:${MATILDA_RIVE_LIB_DIR}/rive_decoders.lib"
            "/WHOLEARCHIVE:${MATILDA_RIVE_LIB_DIR}/rive.lib"
        )
    endif()
    target_link_libraries(Matilda PUBLIC ${MATILDA_RIVE_LIBS})
endif()
