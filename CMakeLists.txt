cmake_minimum_required(VERSION 3.22)

project(FaseGroovePiano VERSION 1.24.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Universal binary (Apple Silicon + Intel) on macOS
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

option(SGK_BUILD_PLUGIN "Build the JUCE plug-in (needs JUCE)" ON)
option(SGK_BUILD_TESTS  "Build the engine unit tests" ON)

#==============================================================================
# Music engine (pure C++, no JUCE)
add_library(sgk_engine STATIC
    Source/engine/Theory.cpp
    Source/engine/GrooveEngine.cpp
    Source/engine/Generators.cpp
    Source/engine/HarmonyAnalyzer.cpp)
target_include_directories(sgk_engine PUBLIC Source/engine)
set_target_properties(sgk_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(SGK_BUILD_TESTS)
    enable_testing()
    add_executable(sgk_engine_tests tests/EngineTests.cpp)
    target_link_libraries(sgk_engine_tests PRIVATE sgk_engine)
    add_test(NAME engine COMMAND sgk_engine_tests)
endif()

#==============================================================================
# Plug-in
if(SGK_BUILD_PLUGIN)
    # Use a local JUCE checkout with -DJUCE_DIR=/path/to/JUCE, otherwise download it.
    set(JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout")

    if(JUCE_DIR)
        add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)
    else()
        include(FetchContent)
        FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.9
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(JUCE)
    endif()

    set(SGK_FORMATS VST3 Standalone)
    if(APPLE)
        list(APPEND SGK_FORMATS AU)
    endif()

    juce_add_plugin(FaseGroovePiano
        PRODUCT_NAME "Fase Groove Piano"
        COMPANY_NAME "FS Pitch"
        BUNDLE_ID "com.fspitch.fasegroovepiano"
        PLUGIN_MANUFACTURER_CODE FsPt
        PLUGIN_CODE Fgp1
        ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon_512.png"
        FORMATS ${SGK_FORMATS}
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_CATEGORIES Instrument Generator
        AU_MAIN_TYPE kAudioUnitType_MusicDevice)

    juce_add_binary_data(FaseGroovePianoData SOURCES
        Resources/fgp_icon.svg
        Resources/fs_pitch_logo.png)

    target_sources(FaseGroovePiano PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/CircleOfFifths.cpp
        Source/Branding.cpp
        Source/Localisation.cpp
        Source/Updater.cpp)

    target_compile_definitions(FaseGroovePiano PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

    target_link_libraries(FaseGroovePiano
        PUBLIC
            FaseGroovePianoData
        PRIVATE
            sgk_engine
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

    # Plug-in level tests (state, presets, MIDI export, transport, updates); run by ctest in CI.
    if(SGK_BUILD_TESTS)
        juce_add_console_app(sgk_plugin_tests PRODUCT_NAME "Fase Groove Piano Tests")
        target_sources(sgk_plugin_tests PRIVATE tests/PluginTests.cpp)
        target_link_libraries(sgk_plugin_tests PRIVATE FaseGroovePiano juce::juce_audio_utils juce::juce_dsp)
        add_test(NAME plugin COMMAND sgk_plugin_tests)
    endif()
endif()
