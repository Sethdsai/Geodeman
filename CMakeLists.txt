cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Pathfinder VERSION 1.0.0)

file(GLOB_RECURSE PATHFINDER_SOURCES
    src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${PATHFINDER_SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Set the GEODE_SDK environment variable to your Geode SDK install (https://docs.geode-sdk.org/).")
endif()

include($ENV{GEODE_SDK}/cmake/CPM.cmake)
include($ENV{GEODE_SDK}/cmake/Geode.cmake)

target_include_directories(${PROJECT_NAME} PRIVATE src)

setup_geode_mod(${PROJECT_NAME})
