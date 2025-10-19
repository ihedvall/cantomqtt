# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

find_package(Python REQUIRED COMPONENTS Interpreter)
cmake_print_variables(Python_FOUND Python_EXECUTABLE)
if (Python_FOUND)
    execute_process( COMMAND ${Python_EXECUTABLE}
            -m pip install --upgrade pip  )
    execute_process( COMMAND ${Python_EXECUTABLE}
            -m pip install mkdocs --upgrade )
    execute_process( COMMAND ${Python_EXECUTABLE}
            -m pip install mkdocs-material --upgrade)
endif()