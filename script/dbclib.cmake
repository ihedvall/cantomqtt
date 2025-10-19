# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)

FetchContent_Declare(dbclib
        GIT_REPOSITORY https://github.com/ihedvall/dbclib.git
        GIT_TAG HEAD)

set(DBC_DOC OFF)
set(DBC_TEST OFF)
set(DBC_TOOLS OFF)
set(DBC_PYTHON OFF)
set(DBC_FLEX OFF)

FetchContent_MakeAvailable(dbclib)

cmake_print_variables(
        dbclib_POPULATED
        dbclib_SOURCE_DIR
        dbclib_BINARY_DIR )

cmake_print_properties(TARGETS dbc PROPERTIES INCLUDE_DIRECTORIES)