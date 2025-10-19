# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

project(CanToMqttApp
        VERSION 1.0
        DESCRIPTION "Simple executable for the CAN to MQTT application"
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

add_executable(can-to-mqtt-app  src/main.cpp)

if (MSVC)
    target_compile_definitions(can-to-mqtt-app PRIVATE -D_WIN32_WINNT=0x0A00)
endif ()

target_link_libraries(can-to-mqtt-app PRIVATE can-to-mqtt-lib)
target_link_libraries(can-to-mqtt-app PRIVATE metric-lib)
target_link_libraries(can-to-mqtt-app PRIVATE mqtt-metric-lib)
target_link_libraries(can-to-mqtt-app PRIVATE bus-message-lib)
target_link_libraries(can-to-mqtt-app PRIVATE bus-message-interface)
target_link_libraries(can-to-mqtt-app PRIVATE dbc)
target_link_libraries(can-to-mqtt-app PRIVATE util)
target_link_libraries(can-to-mqtt-app PRIVATE Boost::filesystem)
target_link_libraries(can-to-mqtt-app PRIVATE Boost::process)
target_link_libraries(can-to-mqtt-app PRIVATE EXPAT::EXPAT)
target_link_libraries(can-to-mqtt-app PRIVATE eclipse-paho-mqtt-c::paho-mqtt3a-static)



