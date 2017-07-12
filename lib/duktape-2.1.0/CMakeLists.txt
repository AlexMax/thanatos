cmake_minimum_required(VERSION 3.2)

project(duktape VERSION "2.1.0" LANGUAGES C)

add_library(duktape
                duk_config.h
    duktape.c   duktape.h)
set_property(TARGET duktape PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${PROJECT_SOURCE_DIR}")
