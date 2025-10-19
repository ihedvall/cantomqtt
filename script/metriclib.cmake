# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)

FetchContent_Declare(metriclib
        GIT_REPOSITORY https://github.com/ihedvall/metriclib.git
        GIT_TAG HEAD)

set(METRIC_DOC OFF)
set(METRIC_TEST OFF)
set(METRIC_TOOLS OFF)
set(METRIC_MQTT ON)
FetchContent_MakeAvailable(metriclib)

cmake_print_variables(
        metriclib_POPULATED
        metriclib_SOURCE_DIR
        metriclib_BINARY_DIR )

cmake_print_properties(TARGETS metric-lib PROPERTIES INCLUDE_DIRECTORIES)
