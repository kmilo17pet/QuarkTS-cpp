cmake_minimum_required(VERSION 3.2)

project(test_quarkts_cpp)

add_subdirectory( src )
include_directories( src )

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

target_include_directories( ${PROJECT_NAME} PUBLIC PROJECT_SOURCE_DIR )

target_compile_options( ${PROJECT_NAME} PRIVATE
           $<$<COMPILE_LANGUAGE:CXX>:
               -std=c++11 -Wall -Wextra -Wpedantic -Wconversion
               -flto -Wimplicit-fallthrough=0 -Wformat-security
               -Wduplicated-cond -Wfloat-equal -Wshadow -Wsign-conversion
               -Wlogical-not-parentheses -Wnull-dereference  -Wstringop-overflow 
               -fdump-rtl-expand -fstrict-aliasing  -Wstrict-aliasing 
               -Wparentheses -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs
               -Wunused -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wshadow 
               -Wwrite-strings -Wsign-compare  -Wmissing-declarations -Wformat 
               -Wmissing-format-attribute -Wno-deprecated-declarations -Wpacked 
               -Wredundant-decls -Wunreachable-code
            >
)
add_executable ( simple_dry_test check/simple_dry_test.cpp )
target_link_libraries( simple_dry_test quarkts-cpp )
