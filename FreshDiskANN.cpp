//
// Created by Juan Pedro on 22/11/2024.
//
#include <cmath>
#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include "FreshDiskANN.h"

const std::string FreshDiskANN::DEFAULT_FILE_PATH_PRECISION_LTI = "precisionLTI.dat";
const std::string FreshDiskANN::DEFAULT_FILE_PATH_INTERMEDIATE_LTI = "intermediateLTI.dat";

const size_t FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND = 10;
const double FreshDiskANN::DEFAULT_ALPHA = 1.2;
const double FreshDiskANN::DEFAULT_DELETE_ACCUMULATION_FACTOR = 0.1;
const size_t FreshDiskANN::DEFAULT_SEARCH_LIST_SIZE = 10;

//Constructores de los componentes
PrecisionLTI precisionLTI(FreshDiskANN::DEFAULT_FILE_PATH_PRECISION_LTI, FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND); //SSD
//CompressedLTI todavia no tiene constructor
std::shared_ptr<FreshVamanaIndex> roTempIndex = std::make_shared<FreshVamanaIndex>(FreshDiskANN::DEFAULT_ALPHA, FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND, FreshDiskANN::DEFAULT_DELETE_ACCUMULATION_FACTOR);
std::shared_ptr<FreshVamanaIndex> rwTempIndex = std::make_shared<FreshVamanaIndex>(FreshDiskANN::DEFAULT_ALPHA, FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND, FreshDiskANN::DEFAULT_DELETE_ACCUMULATION_FACTOR);

//Metodos auxiliares

double FreshDiskANN::distance(std::shared_ptr<GraphNode> node, std::shared_ptr<GraphNode> xq) {
    double sum = 0.0;
    for (size_t i = 0; i < node->features.size(); ++i) {
        sum += std::pow(node->features[i] - xq->features[i], 2);
    }
    return std::sqrt(sum);
}

void FreshDiskANN::robustPrune(std::shared_ptr<GraphNode> p, std::vector<std::shared_ptr<GraphNode>> &v, double alpha, size_t outDegreeBound) {
    //v ← (v ∪ 𝑁out(𝑝)) \ {𝑝}
    for (auto neighbor: p->outNeighbors) {
        if (std::find(v.begin(), v.end(), neighbor) == v.end()) {
            v.push_back(neighbor);
        }
    }
    v.erase(std::remove(v.begin(), v.end(), p), v.end());

    //𝑁out(𝑝) <- ∅
    p->outNeighbors.clear();

    //while v != ∅:
    while (!v.empty()) {
        //p* <- argminp'∈𝑣{d(p, 𝑝')}
        auto minIt = std::min_element(v.begin(), v.end(),
                                      [&](std::shared_ptr<GraphNode> a, std::shared_ptr<GraphNode> b) {
                                          return distance(p, a) < distance(p, b);
                                      });
        std::shared_ptr<GraphNode> pStar = *minIt;

        //𝑁out(𝑝) <- 𝑁out(𝑝) ∪ {p*}
        p->outNeighbors.push_back(pStar);

        //if |𝑁out(𝑝)| = outDegreeBound: break
        if (p->outNeighbors.size() == outDegreeBound) {
            break;
        }

        //for p' ∈ 𝑣:
        for (auto it = v.begin(); it != v.end();) {
            std::shared_ptr<GraphNode> pPrime = *it;
            //if d(p*, p') ≤ d(p,p')/alpha then remove p' from v
            if (distance(pStar, pPrime) <= distance(p, pPrime) / alpha) {
                it = v.erase(it);
            } else {
                ++it;
            }
        }
    }
}


void FreshDiskANN::deleteConsolidation(std::vector<std::shared_ptr<GraphNode>> graph, std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> deleteList) {

    for (auto node: graph) {
        // foreach p in P \ L_D (omit nodes in L_D)
        if (deleteList.find(node) == deleteList.end()) {
            continue;
        }
        // Nout(p) n L_D != {}
        std::set<std::shared_ptr<GraphNode>> deletedNeighbors;
        std::set_intersection(deleteList.begin(), deleteList.end(), node->outNeighbors.begin(),
                              node->outNeighbors.end(), std::inserter(deletedNeighbors, deletedNeighbors.begin()));
        //        auto deletedNeighbors = std::find_if(node->outNeighbors.begin(), node->outNeighbors.end(), [this](std::shared_ptr<GraphNode> outNeighbor){
        //            return deleteList.find(outNeighbor) != deleteList.end();
        //        });
        if (deletedNeighbors.empty()) {
            continue;
        }

        // "C ← 𝑁out(𝑝) \ D" Inicializamos la lista de candidatos
        std::set<std::shared_ptr<GraphNode>> candidateList;
        for (auto outNeighbor: node->outNeighbors) {
            // verify neighbor not in D
            if (deletedNeighbors.find(node) == deletedNeighbors.end()) {
                candidateList.insert(node);
            }
        }

        // "foreach 𝑣 ∈ D do C ← C ∪ 𝑁out(𝑣)"
        for (auto deletedNeighbor: deletedNeighbors) {
            candidateList.insert(deletedNeighbor->outNeighbors.begin(), deletedNeighbor->outNeighbors.end());
        }

        // "C ← C \ D"
        std::set<std::shared_ptr<GraphNode>> finalCandidateList;
        std::set_difference(candidateList.begin(), candidateList.end(), deletedNeighbors.begin(),
                            deletedNeighbors.end(), std::inserter(finalCandidateList, finalCandidateList.begin()));
        auto candidates = std::vector(finalCandidateList.begin(), finalCandidateList.end());
        robustPrune(node, candidates, alpha, outDegreeBound);
    }
}


//Metodos
void FreshDiskANN::insert(std::shared_ptr<GraphNode> p, size_t searchListSize, bool chooseRandom){

    //Insertar en el RW-TempIndex
    std::cout<<"Insertando el nodo:" << p->id << " en el RW-TempIndex" << std::endl;
    rwTempIndex->insert(p, FreshDiskANN::DEFAULT_SEARCH_LIST_SIZE, false);

    //Insertar en el PrecisionLTI (TEST) TODO:BORRAR LUEGO DE TESTEAR
    std::cout<<"Insertando el nodo:" << p->id << " en el PrecisionLTI" << std::endl;
    precisionLTI->storeNode(p);


    std::cout<<"Insertando el nodo:" << p->id << " en el CompressedLTI" << std::endl;
    //Insertar en el CompressedLTI
    compressedLTI.PQandStoreNode(p, 5);


}

void FreshDiskANN::deleteNode(std::shared_ptr<GraphNode> p) {
    p -> deleted=true; //mark as deleted
    deleteList.insert(p); //add to delete list
}

void FreshDiskANN::deletePhase(size_t maxBlockSize) {

    size_t totalNodes = *(precisionLTI->getNodeCount());
    size_t nodesPerBlock = maxBlockSize / 4096; // Each node is 4kB
    size_t totalBlocks = (totalNodes + nodesPerBlock - 1) / nodesPerBlock; // Ceiling division

    std::cout << "Total Nodes: " << totalNodes << std::endl;
    std::cout << "Nodes per Block: " << nodesPerBlock << std::endl;
    std::cout << "Total Blocks: " << totalBlocks << std::endl;

    std::ifstream file(FreshDiskANN::DEFAULT_FILE_PATH_PRECISION_LTI, std::ios::binary);

    if (!file) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }

    std::ofstream intermediateFile(FreshDiskANN::DEFAULT_FILE_PATH_INTERMEDIATE_LTI, std::ios::binary);

    if (!intermediateFile) {
        std::cerr << "Error creating intermediate file" << std::endl;
        return;
    }

    //Esto es lo que se puede paralelizar, dandole un block index a cada thread (y por lo tanto tengan su propio StartNode y EndNode)
    for (size_t blockIndex = 0; blockIndex < totalBlocks; ++blockIndex) {
        std::cout << "====================================" << std::endl;
        std::cout << "Block Index: " << blockIndex << std::endl;
        std::cout << "====================================" << std::endl;

        size_t startNode = blockIndex * nodesPerBlock;
        size_t endNode = std::min(startNode + nodesPerBlock, totalNodes);

        std::cout << "Start Node: " << startNode << std::endl;
        std::cout << "End Node: " << endNode << std::endl<<std::endl;

        std::vector<std::shared_ptr<GraphNode>> nodesInBlock;

        //Read each GraphNode from the block from the PrecisionLTI
        for (size_t nodeId = startNode; nodeId < endNode; ++nodeId) {
            file.seekg(nodeId * 4096, std::ios::beg);

            int id;
            size_t featureSize, neighborCount;
            std::vector<double> features;
            std::vector<int> neighborIds;

            //Read node ID
            file.read(reinterpret_cast<char*>(&id), sizeof(id));

            //Read feature size
            file.read(reinterpret_cast<char*>(&featureSize), sizeof(featureSize));
            features.resize(featureSize);

            //Read features
            file.read(reinterpret_cast<char*>(features.data()), featureSize * sizeof(double));

            //Read neighbor count
            file.read(reinterpret_cast<char*>(&neighborCount), sizeof(neighborCount));
            neighborIds.resize(neighborCount);

            //Read neighbor IDs
            for (size_t i = 0; i < neighborCount; ++i) {
                file.read(reinterpret_cast<char*>(&neighborIds[i]), sizeof(neighborIds[i]));
            }

            auto node = std::make_shared<GraphNode>(id, features);
            nodesInBlock.push_back(node);

            //Retrieve neighbors
            for (const auto& neighborId : neighborIds) {
                auto neighbor = precisionLTI->retrieveNode(neighborId);
                node->outNeighbors.push_back(neighbor);
            }
        }


        //Execute Algorithm 4 for the nodes in the block using multiple threads
        deleteConsolidation(nodesInBlock, deleteList);

        std::shared_ptr<GraphNode> retrievedNode;

        //Write the modified block back to SSD on the intermediateLTI
        for (auto node : nodesInBlock) {
            intermediateLTI->storeNode(node);

            retrievedNode = precisionLTI->retrieveNode(node->id);

            if (retrievedNode != nullptr) {
                std::cout << "PrecisionLTI: Node ID: " << retrievedNode->id << " | ";
                /*
                std::cout << "Features: ";
                for (const auto& feature : currentNode->features) {
                    std::cout << feature << " ";
                }
                */
                std::cout << "Out-Neighbors: ";
                for (const auto& neighbor : retrievedNode->outNeighbors) {
                    std::cout << neighbor->id << " ";
                }
                std::cout << std::endl;
            }

            retrievedNode = intermediateLTI->retrieveNode(node->id);

            if (retrievedNode != nullptr) {
                std::cout << "intermediateLTI: Node ID: " << retrievedNode->id << " | ";
                /*
                std::cout << "Features: ";
                for (const auto& feature : currentNode->features) {
                    std::cout << feature << " ";
                }
                */
                std::cout << "Out-Neighbors: ";
                for (const auto& neighbor : retrievedNode->outNeighbors) {
                    std::cout << neighbor->id << " ";
                }
                std::cout << std::endl;
            }

            retrievedNode = rwTempIndex->getNode(node->id);
            if (retrievedNode != nullptr) {
                std::cout << "rwTempIndex: Node ID: " << retrievedNode->id << " | ";
                /*
                std::cout << "Features: ";
                for (const auto& feature : currentNode->features) {
                    std::cout << feature << " ";
                }
                */
                std::cout << "Out-Neighbors: ";
                for (const auto& neighbor : retrievedNode->outNeighbors) {
                    std::cout << neighbor->id << " ";
                }
                std::cout << std::endl;
            }


            std::cout<<"============================================="<<std::endl;

        }



    }

    file.close();
}


void FreshDiskANN::streamingMerge() {
    std::cout << "Stating with the delete phase" << std::endl;
    deletePhase();
}