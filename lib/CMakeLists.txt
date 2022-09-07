cmake_minimum_required(VERSION 3.19)

if (NOT DEFINED PROJECT_NAME)
    set(LIBSMBUSB_NOT_SUBPROJECT ON)
endif()

project(libsmbusb)

include(GNUInstallDirs)

# where to look first for cmake modules, before ${CMAKE_ROOT\}/modules/ is checked
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

find_package(Threads)
#find_package(PkgConfig)
find_package(LIBUSB1 REQUIRED)

# Warnings
set (LIBSMBUSB_WARNING_OPTIONS
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
       -Wall -Wextra>
     $<$<CXX_COMPILER_ID:MSVC>:
       /W4>)

#aux_source_directory(. SRC_LIST)
#add_executable(${PROJECT_NAME} ${SRC_LIST})
#target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
#target_compile_options(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_WARNING_OPTIONS})

set(LIBSMBUSB_HDR
    firmware.h
    fxloader.h
    libsmbusb.h
    )
set(LIBSMBUSB_SRC
    fxloader.c
    smbusb-i2cdev.c
    #smbusb.c
    )

add_library(smbusb ${LIBSMBUSB_SRC} ${LIBSMBUSB_HDR})
target_link_libraries(smbusb PRIVATE LIBUSB1::LIBUSB1)
target_compile_options(smbusb PRIVATE ${LIBSMBUSB_WARNING_OPTIONS})
target_include_directories(smbusb
    PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/avcpp>
    PRIVATE
          ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(smbusb PROPERTIES PUBLIC_HEADER "libsmbusb.h")
add_library(smbusb::smbusb ALIAS smbusb)


# Install
if (LIBSMBUSB_NOT_SUBPROJECT)
    install(TARGETS smbusb
        EXPORT smbusb-targets
        LIBRARY DESTINATION        ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION        ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION        ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION       ${CMAKE_INSTALL_INCLUDEDIR}
        PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION  ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT smbusb-targets
        FILE smbusb-targets.cmake
        NAMESPACE smbusb::
        DESTINATION lib/cmake/smbusb)

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
      "Config.cmake.in"
      "smbusb-config.cmake"
      INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smbusb
#      PATH_VARS
#        CMAKE_INSTALL_LIBDIR
      )
    write_basic_package_version_file("smbusb-config-version.cmake"
        VERSION 1.0.0
        COMPATIBILITY SameMajorVersion)
    install(
        FILES
          "${CMAKE_CURRENT_BINARY_DIR}/smbusb-config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/smbusb-config-version.cmake"
        DESTINATION
          "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/smbusb"
      )

      if (PKG_CONFIG_FOUND)
        # TBD
      endif()
endif()

