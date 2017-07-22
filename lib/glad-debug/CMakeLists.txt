cmake_minimum_required(VERSION 3.2)

# Generated with http://glad.dav1d.de/ 
# Actually glad 0.1.14A0, but who is counting...
project(glad-debug VERSION "0.1.14" LANGUAGES C)

add_library(glad-debug
    "src/glad.c"    "include/glad/glad.h"
                    "include/KHR/khrplatform.h")
target_include_directories(glad-debug PUBLIC "${PROJECT_SOURCE_DIR}/include")
