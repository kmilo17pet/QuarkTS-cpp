cmake_minimum_required(VERSION 3.2)

project(test_quarkts_cpp)

add_subdirectory( os )
include_directories( os/include )

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

add_compile_options( -std=c++11 -Wall -Wextra -Wpedantic -ansi -Wconversion 
                     -flto -Wextra -Wimplicit-fallthrough=0 -Wformat-security
                     -Wduplicated-cond -Wfloat-equal -Wshadow -Wconversion 
                     -Wsign-conversion -Wlogical-not-parentheses 
                     -Wnull-dereference  -Wnull-dereference -Wstringop-overflow 
                     -fdump-rtl-expand -fstrict-aliasing  -Wstrict-aliasing 
                   )
add_executable ( simple_dry_test simple_dry_test.cpp )
target_link_libraries( simple_dry_test quarkts-cpp )