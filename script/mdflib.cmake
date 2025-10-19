# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)

FetchContent_Declare(mdflib
        GIT_REPOSITORY https://github.com/ihedvall/mdflib.git
        GIT_TAG HEAD)

set(BUILD_SHARED_LIBS OFF)
set(MDF_BUILD_SHARED_LIB OFF)

FetchContent_MakeAvailable(mdflib)

cmake_print_variables(mdflib_POPULATED mdflib_SOURCE_DIR mdflib_BINARY_DIR )

cmake_print_properties(TARGETS mdf PROPERTIES INCLUDE_DIRECTORIES)