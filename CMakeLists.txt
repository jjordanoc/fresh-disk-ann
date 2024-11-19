cmake_minimum_required(VERSION 3.26)
project(freshdiskann)

set(CMAKE_CXX_STANDARD 17)

add_executable(freshdiskann main.cpp
        GraphNode.cpp
        GraphNode.h
        Index.cpp
        Index.h)
