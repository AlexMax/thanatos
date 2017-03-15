# - Find SDL2_net library and headers
# 
# Find module for SDL_net 2.0 (http://www.libsdl.org/projects/SDL_net/).
# This module defines:
#  SDL2_NET_FOUND - If false, do not try to use SDL2_net.
#  SDL2_NET_VERSION_STRING
#    Human-readable string containing the version of SDL2_net.
#  SDL2::net, the import target for SDL2_net
#
#=============================================================================
# Copyright 2013 Benjamin Eikel
# Copyright 2017 Alex Mayfield
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

add_library(SDL2::net UNKNOWN IMPORTED)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SDL2_NET QUIET SDL2_net)

find_path(SDL2_NET_INCLUDE_DIR
  NAMES SDL_net.h
  HINTS
    ${PC_SDL2_NET_INCLUDEDIR}
    ${PC_SDL2_NET_INCLUDE_DIRS}
  PATH_SUFFIXES SDL2
)
if(SDL2_NET_INCLUDE_DIR)
  set_property(TARGET SDL2::net PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${SDL2_NET_INCLUDE_DIR})
endif()

find_library(SDL2_NET_LIBRARY
  NAMES SDL2_net
  HINTS
    ${PC_SDL2_NET_LIBDIR}
    ${PC_SDL2_NET_LIBRARY_DIRS}
  PATH_SUFFIXES x64 x86
)
if(SDL2_NET_LIBRARY)
  set_property(TARGET SDL2::net PROPERTY IMPORTED_LOCATION ${SDL2_NET_LIBRARY})
endif()

if(SDL2_NET_INCLUDE_DIR AND EXISTS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h")
  file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_MAJOR "${SDL2_NET_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_MINOR "${SDL2_NET_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_PATCH "${SDL2_NET_VERSION_PATCH_LINE}")
  set(SDL2_NET_VERSION_STRING ${SDL2_NET_VERSION_MAJOR}.${SDL2_NET_VERSION_MINOR}.${SDL2_NET_VERSION_PATCH})
  unset(SDL2_NET_VERSION_MAJOR_LINE)
  unset(SDL2_NET_VERSION_MINOR_LINE)
  unset(SDL2_NET_VERSION_PATCH_LINE)
  unset(SDL2_NET_VERSION_MAJOR)
  unset(SDL2_NET_VERSION_MINOR)
  unset(SDL2_NET_VERSION_PATCH)
endif()

set(SDL2_NET_INCLUDE_DIRS ${SDL2_NET_INCLUDE_DIR})
set(SDL2_NET_LIBRARIES ${SDL2_NET_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL2_net
                                  REQUIRED_VARS SDL2_NET_INCLUDE_DIRS SDL2_NET_LIBRARIES
                                  VERSION_VAR SDL2_NET_VERSION_STRING)
