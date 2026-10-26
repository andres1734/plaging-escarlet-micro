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
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE AnVo
    PLUGIN_CODE ScWm
    FORMATS VST3 Standalone
    PRODUCT_NAME "Scarlett Warmth Preamp"
)

target_sources(ScarlettWarmth
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
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

# Automatic copy to Windows FL Studio VST3 Common Files Directory
if(WIN32)
    set(COMMON_VST3_DIR "$ENV{COMMONPROGRAMFILES}/VST3")
    add_custom_command(TARGET ScarlettWarmth_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${COMMON_VST3_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:ScarlettWarmth_VST3>/Scarlett Warmth Preamp.vst3"
            "${COMMON_VST3_DIR}/Scarlett Warmth Preamp.vst3"
        COMMENT "Copiando Scarlett Warmth Preamp.vst3 directamente a la carpeta de FL Studio: ${COMMON_VST3_DIR}"
    )
endif()
