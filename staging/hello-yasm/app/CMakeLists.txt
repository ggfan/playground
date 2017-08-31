cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Q1: why this is NASM, but not YASM?
enable_language(ASM_NASM)

set_property(SOURCE src/main/cpp/yasm_compute.asm
    APPEND_STRING PROPERTY COMPILE_FLAGS "-f elf64")

add_library(native-lib SHARED
    src/main/cpp/native-lib.cpp
    src/main/cpp/yasm_compute.asm)
find_library(log-lib log )
target_link_libraries(native-lib
                      ${log-lib} )