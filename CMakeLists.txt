cmake_minimum_required(VERSION 3.26)
project(freshdiskann)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

add_executable(freshdiskann main.cpp
        GraphNode.cpp
        GraphNode.h
        FreshVamanaIndex.cpp
        FreshVamanaIndex.h
        FreshVamanaTestUtils.hpp
        Utils.hpp)
