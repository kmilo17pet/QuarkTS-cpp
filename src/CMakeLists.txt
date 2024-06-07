cmake_minimum_required( VERSION 3.2 )
project( quarkts-cpp
         VERSION 1.6.0
         DESCRIPTION "An open-source OS for small embedded applications"
         LANGUAGES CXX )

add_library( ${PROJECT_NAME} INTERFACE )

target_sources( ${PROJECT_NAME} INTERFACE
             "${CMAKE_CURRENT_LIST_DIR}/critical.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/list.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/clock.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/timer.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/queue.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/task.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/prioqueue.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/kernel.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/fsm.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/coroutine.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/util.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/cli.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/memory.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/bytebuffer.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/response.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/input.cpp"
             "${CMAKE_CURRENT_LIST_DIR}/logger.cpp"
           )

target_compile_options( ${PROJECT_NAME} INTERFACE
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
target_include_directories( ${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR} )

