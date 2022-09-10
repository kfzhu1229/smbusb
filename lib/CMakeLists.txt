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

find_package(PkgConfig)

set(ENABLE_FX2LP_PROGRAMMER On CACHE BOOL "Enable FX2LP programmer")

# i2c-dev is a Linux-specific
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(ENABLE_I2CDEV_PROGRAMMER On CACHE BOOL "Enable generic i2c-dev programmers")
endif()

# Warnings
set (LIBSMBUSB_WARNING_OPTIONS
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
       -Wall -Wextra>
     $<$<CXX_COMPILER_ID:MSVC>:
       /W4>)

# Dummy libs
set(LIBS m)

set(LIBSMBUSB_HDR libsmbusb.h smbusb_priv.h)
set(LIBSMBUSB_SRC smbusb.c)

if (ENABLE_FX2LP_PROGRAMMER)
    find_package(LIBUSB1 REQUIRED)
    list(APPEND LIBS LIBUSB1::LIBUSB1)
    list(APPEND LIBSMBUSB_HDR
        fxloader.h)
    list(APPEND LIBSMBUSB_SRC smbusb-fx2lp.c fxloader.c)
endif()

if (ENABLE_I2CDEV_PROGRAMMER)
    list(APPEND LIBSMBUSB_SRC smbusb-i2cdev.c)
endif()

add_library(smbusb ${LIBSMBUSB_SRC} ${LIBSMBUSB_HDR})
target_link_libraries(smbusb PRIVATE ${LIBS})
target_compile_options(smbusb PRIVATE ${LIBSMBUSB_WARNING_OPTIONS})
target_include_directories(smbusb
    PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
          ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(smbusb PROPERTIES PUBLIC_HEADER "libsmbusb.h")
add_library(smbusb::smbusb ALIAS smbusb)

if (ENABLE_FX2LP_PROGRAMMER)
    target_compile_definitions(smbusb PRIVATE USE_FX2LP_PROGRAMMER=1)
endif()

if (ENABLE_I2CDEV_PROGRAMMER)
    target_compile_definitions(smbusb PRIVATE USE_I2CDEV_PROGRAMMER=1)
endif()

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

