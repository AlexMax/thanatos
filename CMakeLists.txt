cmake_minimum_required(VERSION 3.2)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

if(NOT CMAKE_GENERATOR MATCHES "^Visual Studio.+" AND NOT CMAKE_GENERATOR STREQUAL Xcode)
    if(DEFINED CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
    else()
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
    endif()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

project("Chocolate Doom" VERSION "3.0.0" LANGUAGES C)

set(PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "chocolate-doom")
set(PACKAGE_BUGREPORT "chocolate-doom-dev-list@chocolate-doom.org")
set(PACKAGE_COPYRIGHT "Copyright (C) 1993-2017")
set(PACKAGE_LICENSE "GNU General Public License, version 2")

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
else()
    # If this is gcc, we have some options we'd like to turn on.  Turn on 
    # optimisation and debugging symbols.
    add_compile_options(-Wall -Wdeclaration-after-statement -Wredundant-decls)
endif()

find_package(SDL2 2.0.1)
find_package(SDL2_mixer 2.0.0)
find_package(SDL2_net 2.0.0)

find_package(PNG)
find_package(Samplerate)

# TODO: We currently link everything against libraries that don't need it.
# Use the specific library CFLAGS/LIBS variables instead of setting them here.
#include_directories(${SDL2_INCLUDE_DIR})
#add_definitions(${SDL2_DEFINITIONS})
if(PNG_FOUND)
    set(HAVE_LIBPNG 1)
    add_library(png UNKNOWN IMPORTED)
    set_property(TARGET png PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        ${PNG_INCLUDE_DIRS})
    set_property(TARGET png PROPERTY INTERFACE_COMPILE_DEFINITIONS
        ${PNG_DEFINITIONS})
    set_property(TARGET png PROPERTY IMPORTED_LOCATION ${PNG_LIBRARY})
    set_property(TARGET png PROPERTY INTERFACE_LINK_LIBRARIES ${PNG_LIBRARIES})
endif()
if(SAMPLERATE_FOUND)
    set(HAVE_LIBSAMPLERATE 1)
    include_directories(${SAMPLERATE_INCLUDE_DIR})
    add_definitions(${SAMPLERATE_DEFINITIONS})
endif()
check_library_exists(m log "" HAVE_LIBM)

set(WINDOWS_RC_VERSION
    "${PROJECT_VERSION_MAJOR}, ${PROJECT_VERSION_MINOR}, ${PROJECT_VERSION_PATCH}, 0")

# This controls the prefix added to the start of program names.  For example,
# if this is changed to "lemon-", the programs generated will be named
# lemon-doom, lemon-heretic, etc.

set(PROGRAM_PREFIX "chocolate-"
    CACHE STRING "Change this when you create your awesome forked version")

configure_file(config.h.in config.h)

foreach(SUBDIR midiproc src)
    add_subdirectory(${SUBDIR})
endforeach()

if(NOT SDL2_FOUND OR NOT SDL2_MIXER_FOUND OR NOT SDL2_NET_FOUND)
    message(FATAL_ERROR "SDL, SDL_mixer, and SDL_net are all required to compile ${CMAKE_PROJECT_NAME}.")
endif()
