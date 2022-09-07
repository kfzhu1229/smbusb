# - try to find libusb-1 library
#
# Cache Variables: (probably not for direct use in your scripts)
#  LIBUSB1_LIBRARY
#  LIBUSB1_INCLUDE_DIR
#
# Non-cache variables you should use in your CMakeLists.txt:
#  LIBUSB1_LIBRARIES
#  LIBUSB1_INCLUDE_DIRS
#  LIBUSB1_FOUND - if this is not true, do not attempt to use this library
#
# Requires these CMake modules:
#  ProgramFilesGlob
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

#[=======================================================================[.rst:
FindLIBUSB1
-----------

Find the native libusb-1.0 includes and library.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines :prop_tgt:`IMPORTED` target ``LIBUSB1::LIBUSB1``, if
libusb-1.0 has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

::

  LIBUSB1_INCLUDE_DIRS   - where to find libusb.h, etc.
  LIBUSB1_LIBRARIES      - List of libraries when using libusb1.
  LIBUSB1_FOUND          - True if libusb1 found.

::

  ZLIB_VERSION_STRING - The version of zlib found (x.y.z)
  ZLIB_VERSION_MAJOR  - The major version of zlib
  ZLIB_VERSION_MINOR  - The minor version of zlib
  ZLIB_VERSION_PATCH  - The patch version of zlib
  ZLIB_VERSION_TWEAK  - The tweak version of zlib

Backward Compatibility
^^^^^^^^^^^^^^^^^^^^^^

The following variable are provided for backward compatibility

::

  ZLIB_MAJOR_VERSION  - The major version of zlib
  ZLIB_MINOR_VERSION  - The minor version of zlib
  ZLIB_PATCH_VERSION  - The patch version of zlib

Hints
^^^^^

A user may set ``LIBUSB1_ROOT`` to a libusb1 installation root to tell this
module where to look.
#]=======================================================================]


set(LIBUSB1_ROOT
	"${LIBUSB1_ROOT}"
	CACHE
	PATH
	"Root directory to search for libusb-1")

if(WIN32)
	include(ProgramFilesGlob)
	program_files_fallback_glob(_dirs "LibUSB-Win32")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		if(MSVC)
			set(_lib_suffixes lib/msvc_x64 MS64/static)
		endif()
	else()
		if(MSVC)
			set(_lib_suffixes lib/msvc MS32/static)
		elseif(COMPILER_IS_GNUCXX)
			set(_lib_suffixes lib/gcc)
		endif()
	endif()
else()
	set(_lib_suffixes)
	find_package(PkgConfig QUIET)
	if(PKG_CONFIG_FOUND)
		pkg_check_modules(PC_LIBUSB1 libusb-1.0)
	endif()
endif()

find_path(LIBUSB1_INCLUDE_DIR
	NAMES
	libusb.h
	PATHS
	${PC_LIBUSB1_INCLUDE_DIRS}
	${PC_LIBUSB1_INCLUDEDIR}
	${_dirs}
	HINTS
	"${LIBUSB1_ROOT}"
	PATH_SUFFIXES
	include/libusb-1.0
	include
	libusb-1.0)

find_library(LIBUSB1_LIBRARY
	NAMES
	libusb-1.0
	usb-1.0
	PATHS
	${PC_LIBUSB1_LIBRARY_DIRS}
	${PC_LIBUSB1_LIBDIR}
	${_dirs}
	HINTS
	"${LIBUSB1_ROOT}"
	PATH_SUFFIXES
	${_lib_suffixes})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBUSB1
	DEFAULT_MSG
	LIBUSB1_LIBRARY
	LIBUSB1_INCLUDE_DIR)

if(LIBUSB1_FOUND)
	set(LIBUSB1_LIBRARIES "${LIBUSB1_LIBRARY}")

	set(LIBUSB1_INCLUDE_DIRS "${LIBUSB1_INCLUDE_DIR}")

	mark_as_advanced(LIBUSB1_ROOT)
	
	if(NOT TARGET LIBUSB1::LIBUSB1)
      add_library(LIBUSB1::LIBUSB1 UNKNOWN IMPORTED)
      set_target_properties(LIBUSB1::LIBUSB1 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LIBUSB1_INCLUDE_DIRS}")

      #if(ZLIB_LIBRARY_RELEASE)
      #  set_property(TARGET LIBUSB1::LIBUSB1 APPEND PROPERTY
      #    IMPORTED_CONFIGURATIONS RELEASE)
      #  set_target_properties(LIBUSB1::LIBUSB1 PROPERTIES
      #    IMPORTED_LOCATION_RELEASE "${LIBUSB1_LIBRARY_RELEASE}")
      #endif()

      #if(ZLIB_LIBRARY_DEBUG)
      #  set_property(TARGET LIBUSB1::LIBUSB1 APPEND PROPERTY
      #    IMPORTED_CONFIGURATIONS DEBUG)
      #  set_target_properties(LIBUSB1::LIBUSB1 PROPERTIES
      #    IMPORTED_LOCATION_DEBUG "${LIBUSB1}")
      #endif()

      #if(NOT ZLIB_LIBRARY_RELEASE AND NOT ZLIB_LIBRARY_DEBUG)
        set_property(TARGET LIBUSB1::LIBUSB1 APPEND PROPERTY
          IMPORTED_LOCATION "${LIBUSB1_LIBRARY}")
      #endif()
    endif()
endif()

mark_as_advanced(LIBUSB1_INCLUDE_DIR LIBUSB1_LIBRARY)
