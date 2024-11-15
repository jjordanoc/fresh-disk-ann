cmake_minimum_required(VERSION 3.26)
project(freshdiskann)

set(CMAKE_CXX_STANDARD 17)

add_executable(freshdiskann main.cpp
        Index.cpp
        Index.h
        GraphNode.cpp
        GraphNode.h
        Vector.cpp
        Vector.h)
