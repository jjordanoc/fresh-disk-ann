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
    p -> deleted=true; //este delete true, sirve mas que nada para que no aparezca en los resultados de busqueda
    //usado mas que nada para que cuando se active en segundo plano el Delete Consolidation del rwTempIndex, el Delete Consolidation sepa a quien borrar.
    deleteList.insert(p); //add to delete list
}

// void FreshDiskANN::deleteNode(size_t id) {
//     deleteList.insert(id);
// }


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



    //Revisames el DeleteList
    // std::cout<<"DeleteList: "<<std::endl;
    // for(auto node : nodesInBlock->deleteList){
    //     std::cout<<node->id<<std::endl;
    // }


    //Esto es lo que se puede paralelizar, dandole un block index a cada thread (y por lo tanto tengan su propio StartNode y EndNode)
    for (size_t blockIndex = 0; blockIndex < totalBlocks; ++blockIndex) {
        std::cout << "====================================" << std::endl;
        std::cout << "Block Index: " << blockIndex << std::endl;
        std::cout << "====================================" << std::endl;

        size_t startNode = blockIndex * nodesPerBlock;
        size_t endNode = std::min(startNode + nodesPerBlock, totalNodes);

        std::cout << "Start Node: " << startNode << std::endl;
        std::cout << "End Node: " << endNode << std::endl<<std::endl;

        std::shared_ptr<FreshVamanaIndex> nodesInBlock = std::make_shared<FreshVamanaIndex>(FreshDiskANN::DEFAULT_ALPHA, FreshDiskANN::DEFAULT_OUT_DEGREE_BOUND, FreshDiskANN::DEFAULT_DELETE_ACCUMULATION_FACTOR);
        nodesInBlock->deleteList = deleteList;

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

            //Tengo que hacer esto, ya que al momento de haber guardado el elemento en el PrecisionLTI,  este todavia no habia sido marcado como eliminado
            if (deleteList.find(node) != deleteList.end()) {
                node->deleted = true;
            }

            //Itero sobre los vecinos
            for (const auto& neighborId : neighborIds) {
                auto neighbor = precisionLTI->retrieveNode(neighborId);

                if (deleteList.find(neighbor) != deleteList.end()) {
                    neighbor->deleted = true;
                }

                //verifico si el vecino no esta en el grafo temporal
                if (nodesInBlock->getNode(neighbor->id) == nullptr) {

                    nodesInBlock->graph.push_back(neighbor);

                    std::cout << "Adding neighbor: " << neighbor->id << " to the block" << std::endl;
                }

                node->outNeighbors.push_back(neighbor);
            }

            std::cout << "Node : " << node->id << " | ";
            std::cout << "Out-Neighbors: ";
            for (const auto& neighbor : node->outNeighbors) {
                std::cout << neighbor->id << " ";
            }
            std::cout << std::endl;





            //Itero sobre los vecinos del nodo y veo primero si ya estan el grafo, de ser asi
            // for (const auto& neighborId : neighborIds) {
            //     auto neighbor = precisionLTI->retrieveNode(neighborId);
            //     //Hago esto, ya que al momento de haber guardado el elemento en el PrecisionLTI,  este todavia no habia sido marcado como eliminado
            //     if (deleteList.find(neighbor) != deleteList.end()) {
            //         neighbor->deleted = true;
            //     }
            //     node->outNeighbors.push_back(neighbor);
            // }

            //nodesInBlock->insert(node); //No puedo hacer insert por que el algoritmo abre los nodos vecinos
            //en este caso solo tengo vecinos de primer nivel. Pero cuando intente abrir los vecinos de mis vecinos
            //no voy a poder.
            //Insertamos el nodo en el grafo temporal

            if (nodesInBlock->getNode(node->id) == nullptr) {
                nodesInBlock->graph.push_back(node);
            }
            else {
                //Reemplazar el nodo
                nodesInBlock->replaceNode(node->id, node);
            }

            //printGraph(nodesInBlock->graph);


            //Liberar nodo
            node.reset();

        }

        printGraph(nodesInBlock->graph);

        //Execute Algorithm 4 for the nodes in the block using multiple threads
        nodesInBlock->deleteConsolidation();

        printGraph(nodesInBlock->graph);

        std::shared_ptr<GraphNode> retrievedNode;

        //Write the modified block back to SSD on the intermediateLTI
        for (const auto& node : nodesInBlock->graph) {

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

void FreshDiskANN::printGraph(std::vector<std::shared_ptr<GraphNode>>& graph) {
    std::cout << std::endl;
    for (auto node : graph) {
        std::cout << node->id;
        if (node->deleted) {
            std::cout << "(D)";
        }
        std::cout << "-> ";
        for (auto outNeighbor : node->outNeighbors) {
            std::cout << outNeighbor->id;
            if (outNeighbor->deleted) {
                std::cout << "(D)";
            }
            std::cout << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


void FreshDiskANN::streamingMerge() {
    std::cout << "Stating with the delete phase" << std::endl;
    deletePhase();
}