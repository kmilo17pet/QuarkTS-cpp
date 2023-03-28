cmake_minimum_required( VERSION 3.2 )
project( quarkts-cpp 
         VERSION 0.0.1 
         DESCRIPTION "An open-source OS for small embedded applications" 
         LANGUAGES CXX )

add_library( ${PROJECT_NAME} STATIC
             critical.cpp
             list.cpp
             clock.cpp
             timer.cpp
             queue.cpp
             task.cpp
             prioqueue.cpp
             kernel.cpp
             fsm.cpp
             coroutine.cpp
             util.cpp
             cli.cpp
             memory.cpp
           )



add_compile_options( -std=c++11 -Wall -Wextra -Wpedantic -ansi -Wconversion 
           -flto -Wimplicit-fallthrough=0 -Wformat-security
           -Wduplicated-cond -Wfloat-equal -Wshadow -Wsign-conversion
           -Wlogical-not-parentheses -Wnull-dereference  -Wstringop-overflow 
           -fdump-rtl-expand -fstrict-aliasing  -Wstrict-aliasing 
         )
set_target_properties( ${PROJECT_NAME}  PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories( ${PROJECT_NAME} PUBLIC include )
target_include_directories( ${PROJECT_NAME} PUBLIC ../config )
target_include_directories( ${PROJECT_NAME} PUBLIC ../os_config )


