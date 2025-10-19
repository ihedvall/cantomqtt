
# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)

FetchContent_Declare(busmessage
        GIT_REPOSITORY https://github.com/ihedvall/bus_message_lib.git
        GIT_TAG HEAD)

set(BUS_DOC OFF)
set(BUS_TEST OFF)
set(BUS_TOOLS ON)
set(BUS_INTERFACE ON)

FetchContent_MakeAvailable(busmessage)

cmake_print_variables(
        busmessage_POPULATED
        busmessage_SOURCE_DIR
        busmessage_BINARY_DIR )