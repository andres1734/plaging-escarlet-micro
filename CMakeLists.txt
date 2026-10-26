cmake_minimum_required(VERSION 3.22)
project(ScarlettWarmth VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags for MSVC (Visual Studio 2022)
if(MSVC)
    add_compile_options(/MP /O2 /fp:fast /W3)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Find or auto-fetch JUCE framework
find_package(JUCE CONFIG QUIET)

if(NOT TARGET juce::juce_dsp)
    include(FetchContent)
    message(STATUS "Descargando JUCE framework para Visual Studio 2022 (solo la primera vez)...")
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.12
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(ScarlettWarmth
    COMPANY_NAME "AnalogVocalDSP"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE  # <-- Desactivado para evitar errores de permisos en CI
    PLUGIN_MANUFACTURER_CODE AnVo
    PLUGIN_CODE ScWm
    FORMATS VST3 Standalone
    PRODUCT_NAME "Scarlett Warmth Preamp"
)

target_sources(ScarlettWarmth
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_definitions(ScarlettWarmth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ScarlettWarmth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
