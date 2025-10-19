# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT EXPAT_FOUND)
    find_package(EXPAT)
    if (NOT EXPAT_FOUND)
        set(EXPAT_USE_STATIC_LIBS ON)
        if (COMP_DIR)
            set(EXPAT_ROOT ${COMP_DIR}/expat/master)
        endif()

        find_package(EXPAT REQUIRED)
    endif()
endif()

cmake_print_variables(
        EXPAT_FOUND
        EXPAT_VERSION_STRING
        EXPAT_INCLUDE_DIRS
        EXPAT_LIBRARY_DIRS
        EXPAT_LIBRARIES )
cmake_print_properties(TARGETS EXPAT::EXPAT PROPERTIES INCLUDE_DIRECTORIES)