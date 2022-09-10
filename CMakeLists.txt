cmake_minimum_required(VERSION 3.19)

if (NOT DEFINED PROJECT_NAME)
    set(SMBUSB_NOT_SUBPROJECT ON)
endif()

project(smbusb)

# Options
set(DISABLE_FIRMWARE ON  CACHE BOOL "Don't build the firmware, use pre-built one")
set(DISABLE_TOOLS    OFF CACHE BOOL "Don't build the tools")

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


add_subdirectory(lib)
if (NOT DISABLE_TOOLS)
    add_subdirectory(tools)
else()
    message(STATUS "Tools will not be built")
endif()

# Build firmware and attach it to the librate pre-build command
if (NOT DISABLE_FIRMWARE)
    find_program(XXD xxd REQUIRED)
    find_program(SDCC sdcc REQUIRED)

    # Check for msc51
    execute_process(
        COMMAND ${SDCC} --version
        COMMAND grep mcs51
        OUTPUT_FILE sdcc_version
        ERROR_FILE sdcc_version
        RESULT_VARIABLE SDCC_MSC51_SUPPORT
    )

    if (NOT ${SDCC_MSC51_SUPPORT} EQUAL 0)
        message(FATAL_ERROR "SDCC does not support msc51 target")
    endif()

    execute_process(COMMAND patch -d ${CMAKE_CURRENT_SOURCE_DIR}/fx2lib -p1 -sN
                    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fx2lib.patch)

    # Target from the lib sub directory
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/lib/firmware.h
        COMMAND make ARGS -C ${CMAKE_CURRENT_SOURCE_DIR}/firmware all
        VERBATIM
    )
    add_custom_target(firmware DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/firmware.h)
    add_dependencies(smbusb firmware)
else()
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/firmware/firmware.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    message(STATUS "Using pre-built firmware")
endif()
