cmake_minimum_required(VERSION 2.8)
project(containers)

# If Debug build, #define DEBUG so we can have it available in code
if(DEFINED CMAKE_BUILD_TYPE)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DDEBUG)
    endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif(DEFINED CMAKE_BUILD_TYPE)

# compile everything in C++14
add_definitions("-std=c++14")

# src/ contains all header files
include_directories(src/)

