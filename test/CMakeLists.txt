# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include(GoogleTest)

project(TestCanToMqtt
        VERSION 1.0
        DESCRIPTION "Unit test for the CAN to MQTT application"
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

add_executable(can-to-mqtt-test
        src/test_cantomqtt.cpp)

target_include_directories(can-to-mqtt-test PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if (MSVC)
    target_compile_definitions(can-to-mqtt-test PRIVATE -D_WIN32_WINNT=0x0A00)
endif ()

target_link_libraries(can-to-mqtt-test PRIVATE can-to-mqtt-lib)
target_link_libraries(can-to-mqtt-test PRIVATE metric-lib)
target_link_libraries(can-to-mqtt-test PRIVATE mqtt-metric-lib)
target_link_libraries(can-to-mqtt-test PRIVATE bus-message-lib)
target_link_libraries(can-to-mqtt-test PRIVATE bus-message-interface)
target_link_libraries(can-to-mqtt-test PRIVATE dbc)
target_link_libraries(can-to-mqtt-test PRIVATE util)
target_link_libraries(can-to-mqtt-test PRIVATE Boost::filesystem)
target_link_libraries(can-to-mqtt-test PRIVATE Boost::process)
target_link_libraries(can-to-mqtt-test PRIVATE EXPAT::EXPAT)
target_link_libraries(can-to-mqtt-test PRIVATE eclipse-paho-mqtt-c::paho-mqtt3a-static)
target_link_libraries(can-to-mqtt-test PRIVATE GTest::gtest_main)


gtest_discover_tests(can-to-mqtt-test)


