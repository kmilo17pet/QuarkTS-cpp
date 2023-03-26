# QuarkTS OS CMakeLists.txt file
# Copyright (C) 2012 Eng. Juan Camilo Gómez Cadavid MSc. All Rights Reserved.
#
# To make use of the QuarkTS OS on your solution just add the following 
# directives to the top-level CMakeLists.txt file :
#
#     add_subdirectory( <relative/path/to/kernel> )
#     target_link_libraries( ${PROJECT_NAME} quarkts )
#
# If the "qconfig.h" header file is not provided, the latest version will be 
# downloaded with the default parameters in the ../os_config directory.
#
# This file is part of the QuarkTS OS distribution.

cmake_minimum_required( VERSION 3.2 )
project( quarkts-cpp 
         VERSION 7.3.2 
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



set_target_properties( ${PROJECT_NAME}  PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories( ${PROJECT_NAME} PUBLIC include )
target_include_directories( ${PROJECT_NAME} PUBLIC ../config )
target_include_directories( ${PROJECT_NAME} PUBLIC ../os_config )


