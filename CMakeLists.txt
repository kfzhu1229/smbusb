cmake_minimum_required(VERSION 3.19)

if (NOT DEFINED PROJECT_NAME)
    set(SMBUSB_NOT_SUBPROJECT ON)
endif()

project(smbusb)

# Options
set(DISABLE_FIRMWARE ON CACHE BOOL "Enable FX2 firmware building (Linux only)")

# where to look first for cmake modules, before ${CMAKE_ROOT}/modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# C++ standard: 98, 11, 14, 17, 20, 23
#   to use per-terget: https://cmake.org/cmake/help/v3.8/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF) # turn on/off GNU or some other extensions

# C standard: 90, 99, 11, 17
#   to use per-terget: https://cmake.org/cmake/help/v3.8/prop_tgt/C_STANDARD.html
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS OFF) # turn on/off GNU or some other extensions

# Warnings
#set (${PROJECT_NAME}_WARNING_OPTIONS
#     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
#       -Wall -Wextra>
#     $<$<CXX_COMPILER_ID:MSVC>:
#       /W4>)

# TBD: check sdcc
if (NOT DISABLE_FIRMWARE)
    find_program(XXD xxd REQUIRED)
    find_program(SDCC sdcc REQUIRED)
    # TBD: sdcc --version 2>&1 | grep "mcs51"

endif()

add_subdirectory(lib)
add_subdirectory(tools)
