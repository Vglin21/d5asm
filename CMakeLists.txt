cmake_minimum_required(VERSION 3.16)
project(dot-5 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(d5asm src/main.c)
target_include_directories(d5asm PRIVATE include)
target_compile_options(d5asm PRIVATE -O2 -flto)
target_link_options(d5asm PRIVATE -flto)

if (WIN32)
    target_link_options(d5asm PRIVATE -municode)
endif()