# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.30)
include(CMakePrintHelpers)

if("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
    set(USE_VCPKG OFF)
else()
    set(USE_VCPKG ON)
endif()

option(BUILD_SHARED_LIBS "Static libraries are preferred" OFF)
option(CAN_TO_MQTT_DOC "Build documentation" OFF)
option(CAN_TO_MQTT_TOOLS "Building executable application" OFF)
option(CAN_TO_MQTT_TEST "Building unit tests" OFF)


if(CAN_TO_MQTT_TOOLS AND USE_VCPKG)
    list(APPEND VCPKG_MANIFEST_FEATURES "tools")
endif()

project(can-to-mqtt
        VERSION 1.0
        DESCRIPTION "CAN to MQTT Library"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_DEBUG_POSTFIX d)

include(script/boost.cmake)
include(script/paho.cmake)
include(script/utillib.cmake)
include(script/busmessage.cmake)
include(script/metriclib.cmake)
include(script/dbclib.cmake)
include(script/expat.cmake)

if (CAN_TO_MQTT_TEST)
    include(script/googletest.cmake)
    include(GoogleTest)
endif()

if (CAN_TO_MQTT_DOC)
    include(script/doxygen.cmake)
    include(script/mkdocs.cmake)
endif()

add_library(can-to-mqtt-lib
        src/cantomqtt.cpp
        include/bus/cantomqtt.h
        src/cantomqttapp.cpp
        include/bus/cantomqttapp.h)

target_include_directories(can-to-mqtt-lib PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${utillib_SOURCE_DIR}/include
        ${busmessage_SOURCE_DIR}/include
        ${metriclib_SOURCE_DIR}/include
        ${dbclib_SOURCE_DIR}/include
        ${PAHO_C_INCLUDE_DIRS}
        )


cmake_print_properties(TARGETS can-to-mqtt-lib PROPERTIES INCLUDE_DIRECTORIES)

if (MSVC)
    # Set target to Windows 10
    target_compile_definitions(can-to-mqtt-lib PRIVATE _WIN32_WINNT=0x0A00)
endif ()


if (CAN_TO_MQTT_TOOLS)
    add_subdirectory(server)
endif ()

if (CAN_TO_MQTT_TEST)
    enable_testing()

    add_subdirectory(test)
endif ()

if (CAN_TO_MQTT_DOC)
    execute_process( COMMAND mkdocs build
                     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()
