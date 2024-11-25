cmake_minimum_required(VERSION 3.26)
project(freshdiskann)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()
# Incluir FAISS
find_package(Threads REQUIRED)


# Agregar rutas específicas de Homebrew
if(APPLE)
include_directories(/opt/homebrew/include) # Incluir encabezados de FAISS
link_directories(/opt/homebrew/lib)   # Incluir bibliotecas de FAISS
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
        FreshVamanaTestUtils.hpp)
# Vincular la biblioteca FAISS (y otros enlaces necesarios como pthread)
target_link_libraries(freshdiskann faiss Threads::Threads)

add_executable(fresh_vamana_recall_test fresh_vamana_recall_test.cpp
        GraphNode.cpp
        GraphNode.h
        FreshVamanaIndex.cpp
        FreshVamanaIndex.h
        FreshVamanaTestUtils.hpp
        Utils.hpp)
# Vincular FAISS también al segundo ejecutable
target_link_libraries(fresh_vamana_recall_test faiss Threads::Threads)
