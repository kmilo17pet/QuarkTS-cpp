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
             bytebuffer.cpp
             response.cpp
             edgecheck.cpp
           )

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
set_target_properties( ${PROJECT_NAME}  PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories( ${PROJECT_NAME} PUBLIC ../src )

