# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include(CMakePrintHelpers)

if (NOT eclipse-paho-mqtt-c_ROOT)
    set(eclipse-paho-mqtt-c_ROOT ${COMP_DIR}/pahomqttc/master)
endif ()

find_package(eclipse-paho-mqtt-c)

get_target_property(PAHO_C_INCLUDE_DIRS eclipse-paho-mqtt-c::paho-mqtt3as-static INTERFACE_INCLUDE_DIRECTORIES)

cmake_print_variables(eclipse-paho-mqtt-c_FOUND
        eclipse-paho-mqtt-c_VERSION
        PAHO_C_INCLUDE_DIRS)

cmake_print_properties(TARGETS eclipse-paho-mqtt-c::paho-mqtt3c-static
        eclipse-paho-mqtt-c::paho-mqtt3a-static
        eclipse-paho-mqtt-c::paho-mqtt3cs-static
        eclipse-paho-mqtt-c::paho-mqtt3as-static
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES INTERFACE_LIBRARIES INTERFACE_LINK_LIBRARIES )
