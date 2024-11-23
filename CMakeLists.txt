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
        FreshDiskANN.cpp
        FreshDiskANN.h
        "FreshDiskANN Components/CompressedLTI.cpp"
        "FreshDiskANN Components/CompressedLTI.h"
        "FreshDiskANN Components/PrecisionLTI.cpp"
        "FreshDiskANN Components/PrecisionLTI.h"
        "FreshDiskANN Components/DeleteList.cpp"
        "FreshDiskANN Components/DeleteList.h"
        "FreshDiskANN Components/RWTempIndex.cpp"
        "FreshDiskANN Components/RWTempIndex.h"
        "FreshDiskANN Components/ROTempIndex.cpp"
        "FreshDiskANN Components/ROTempIndex.h"
        FreshVamanaTestUtils.hpp
        fresh_vamana_recall_test.cpp)
