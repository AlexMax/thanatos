# A simple toolchain containing the proper flags for compiling
# 32-bit binaries on Windows using MinGW.

set(CMAKE_C_FLAGS -m32)
set(CMAKE_RC_FLAGS "-F pe-i386")