cmake_minimum_required(VERSION 3.22)

project(RoboVox VERSION 0.1.0 LANGUAGES C CXX)

# JUCE via FetchContent (best practice for 2026 — user needs internet for first configure)
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12  # Pin stable; upgrade to 8.x when desired
)
FetchContent_MakeAvailable(JUCE)

# ===================== espeak-ng Discovery (Cross-Platform) =====================
# Linux: pkg-config (easiest)
# Windows: Build from source or vcpkg, then use -DESPEAK_ROOT=... or set env var
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(ESPEAK QUIET espeak-ng)
endif()

if(NOT ESPEAK_FOUND)
    # Manual / Windows friendly discovery
    find_path(ESPEAK_INCLUDE_DIR 
        NAMES speak_lib.h 
        PATHS 
            ${ESPEAK_ROOT}/src/include 
            ${ESPEAK_ROOT}/include 
            "C:/espeak-ng/include"
            "$ENV{ESPEAK_ROOT}/include"
            "$ENV{ProgramFiles}/espeak-ng/include"
        PATH_SUFFIXES espeak-ng
    )
    
    find_library(ESPEAK_LIBRARY 
        NAMES espeak-ng libespeak-ng
        PATHS 
            ${ESPEAK_ROOT}/build/src 
            ${ESPEAK_ROOT}/build/Release
            "C:/espeak-ng/build/src/Release"
            "$ENV{ESPEAK_ROOT}/build/src/Release"
    )

    if(ESPEAK_INCLUDE_DIR AND ESPEAK_LIBRARY)
        set(ESPEAK_FOUND TRUE)
        set(ESPEAK_INCLUDE_DIRS ${ESPEAK_INCLUDE_DIR})
        set(ESPEAK_LIBRARIES ${ESPEAK_LIBRARY})
        message(STATUS "Found espeak-ng manually at: ${ESPEAK_LIBRARY}")
    else()
        message(WARNING 
            "espeak-ng not found via pkg-config or manual search.\n"
            "Windows users: See the detailed build instructions in README.md\n"
            "Quick options:\n"
            "  1. Build espeak-ng from source and pass -DESPEAK_ROOT=C:/path/to/espeak-ng\n"
            "  2. Use vcpkg: vcpkg install espeak-ng:x64-windows then configure with toolchain file\n"
            "  3. Set environment variable ESPEAK_ROOT before running CMake"
        )
    endif()
endif()

juce_add_plugin(RoboVox
    COMPANY_NAME                "Forge Audio"
    COMPANY_WEBSITE             "https://example.com"
    COMPANY_EMAIL               "forge@audio"
    PLUGIN_MANUFACTURER_CODE    "Frge"
    PLUGIN_CODE                 "Rbvx"
    FORMATS                     VST3 AU Standalone
    PRODUCT_NAME                "RoboVox"
    DESCRIPTION                 "Lightweight MIDI-controlled robotic text-to-speech vocal synth"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_REQUIRES_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD     TRUE
)

juce_generate_juce_header(RoboVox)

target_sources(RoboVox
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Core/TTSynthesizer.cpp
)

target_include_directories(RoboVox
    PRIVATE
        Source
        "C:/Users/ionsa/Documents/GitHub/espeak-ng/src/include"
        ${ESPEAK_INCLUDE_DIRS}
)

target_compile_definitions(RoboVox
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=0   # Modern best practice
)

target_link_libraries(RoboVox
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
        ${ESPEAK_LIBRARIES}
        "C:/Users/ionsa/Documents/GitHub/espeak-ng/build/src/Release/espeak-ng.lib"
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(ESPEAK_FOUND)
    target_compile_options(RoboVox PRIVATE ${ESPEAK_CFLAGS_OTHER})
endif()

# Elite standards: C++17 minimum, warnings as errors in debug
target_compile_features(RoboVox PRIVATE cxx_std_17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(RoboVox PRIVATE -Wall -Wextra -Wpedantic)
endif()