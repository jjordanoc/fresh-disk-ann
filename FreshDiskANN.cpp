//
// Created by Juan Pedro on 22/11/2024.
//

#include "FreshDiskANN.h"

const std::string FreshDiskANN::DEFAULT_FILE_PATH_PRECISION_LTI = "precision_graph_nodes.dat";
const size_t FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND = 10;
const double FreshDiskANN::DEFAULT_ALPHA = 1.2;
const double FreshDiskANN::DEFAULT_DELETE_ACCUMULATION_FACTOR = 0.1;
const size_t DEFAULT_SEARCH_LIST_SIZE = 10;

//Constructores de los componentes
PrecisionLTI precisionLTI(FreshDiskANN::DEFAULT_FILE_PATH_PRECISION_LTI, FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND); //SSD
//CompressedLTI todavia no tiene constructor
std::shared_ptr<FreshVamanaIndex> roTempIndex = std::make_shared<FreshVamanaIndex>(FreshDiskANN::DEFAULT_ALPHA, FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND, FreshDiskANN::DEFAULT_DELETE_ACCUMULATION_FACTOR);
std::shared_ptr<FreshVamanaIndex> rwTempIndex = std::make_shared<FreshVamanaIndex>(FreshDiskANN::DEFAULT_ALPHA, FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND, FreshDiskANN::DEFAULT_DELETE_ACCUMULATION_FACTOR);

//Metodos
void FreshDiskANN::insert(std::shared_ptr<GraphNode> p, std::shared_ptr<GraphNode> s, size_t searchListSize, double alpha, size_t outDegreeBound) {
    rwTempIndex->insert(p, s, searchListSize, alpha, outDegreeBound);
}
