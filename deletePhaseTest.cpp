//
// Created by Juan Pedro on 24/11/2024.
//
#include "FreshDiskANN.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <set>


void loadDatasetAndStoreNodes(std::string csvPath, size_t maxNeighbours, std::shared_ptr<FreshDiskANN> diskANN, size_t nodesAmount) {
    // Verify if rwTempIndex is properly initialized
    if (diskANN->rwTempIndex == nullptr) {
        std::cout << "rwTempIndex is not properly initialized." << std::endl;
        return;
    }

    std::ifstream file(csvPath);
    if (!file.is_open()) {
        std::cout << "Unable to open file: " << csvPath << std::endl;
        return;
    }

    size_t id = 1;
    std::string line;

    // For random number generation (neighbours)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, nodesAmount);

    while (std::getline(file, line)  && id <= nodesAmount) {
        std::vector<double> features;
        std::string value;
        std::istringstream stream(line);

        while (std::getline(stream, value, ',')) {
            try {
                features.push_back(std::stod(value));
            } catch (const std::invalid_argument &) {
                std::cout << "Invalid number in file: " << value << std::endl;
                throw std::runtime_error("Invalid number in file: " + value);
            }
        }

        if (!features.empty()) {
            std::shared_ptr<GraphNode> node = std::make_shared<GraphNode>(id, features);

            // Assign random neighbors to each node
            std::set<size_t> neighborIds;
            while (neighborIds.size() < maxNeighbours) {
                neighborIds.insert(dis(gen));
            }

            std::cout << "Generator: Node ID: " << node->id << " | ";

            for (const auto &neighborId : neighborIds) {
                if (neighborId != node->id) {
                    std::cout<<neighborId<<" ";
                    std::shared_ptr<GraphNode> neighbor = std::make_shared<GraphNode>(neighborId, std::vector<double>{1});
                    node->outNeighbors.push_back(neighbor);
                }
            }
            std::cout<<std::endl;

            //Guardamos en el PrecisionLTI
            diskANN->precisionLTI->storeNode(node);

            std::shared_ptr<GraphNode> retrievedNode = diskANN->precisionLTI->retrieveNode(node->id);

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

            //Insertamos en el rwTempIndex
            diskANN->rwTempIndex->insert(node);

            retrievedNode = diskANN->rwTempIndex->getNode(node->id);
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

            //Insertamos en CompressedLTI
            diskANN->compressedLTI.PQandStoreNode(node, 5);

            //recogemos el nodo comprimido de CompressedLTI sabiendo que el grafo tiene esta forma:     std::unordered_map<int, std::shared_ptr<CompressedGraphNode>> compressedGraphNodes;// Compressed points
            std::shared_ptr<CompressedGraphNode> compressedNode = diskANN->compressedLTI.compressedGraphNodes[node->id];
            if (compressedNode != nullptr) {
                std::cout << "CompressedLTI: Node ID: " << compressedNode->id << " | ";
                std::cout << "Compressed Features: ";
                for (const auto& feature : compressedNode->compressedFeatures) {
                    std::cout << static_cast<int>(feature) << " ";
                }
                std::cout << std::endl;
            }


            //diskANN->insert(node);
            std::cout<<"============================================="<<std::endl;
            id++;
        }
    }
}
//Test Insertion of a node into FreshDiskANN::rwTempIndex
void testRWIndexInsertion(std::shared_ptr<FreshDiskANN> diskANN) {

    // Verify if rwTempIndex is properly initialized
    if (diskANN->rwTempIndex == nullptr) {
        std::cerr << "rwTempIndex is not properly initialized." << std::endl;
        return;
    } else {
        std::cout << "rwTempIndex is properly initialized." << std::endl;
    }

    // Create a new GraphNode
    auto newNode = std::make_shared<GraphNode>(1, std::vector<double>{0.1, 0.2, 0.3});

    // Insert the new node into rwTempIndex
    diskANN->rwTempIndex->insert(newNode);

}

int main() {

    // Create a new FreshDiskANN object
    std::shared_ptr<FreshDiskANN> diskANN = std::make_shared<FreshDiskANN>();

    //testRWIndexInsertion(diskANN); //PASSED


    std::cout<<"============================================="<<std::endl;
    std::cout<< "Ahora procedemos con la insercion de nodos" << std::endl;
    std::cout<<"============================================="<<std::endl;

    //Insertamos los nodos del Dataset
    loadDatasetAndStoreNodes("/Users/joaquin/Desktop/eda/freshdiskann/cmake-build-relwithdebinfo/siftsmall_base.csv", 10, diskANN, 10);


    std::cout<<"============================================="<<std::endl;
    std::cout<< "Ahora procedemos con la eliminacion de nodos" << std::endl;
    std::cout<<"============================================="<<std::endl;

    //Eliminamos un nodo
    diskANN->deleteNode(diskANN->precisionLTI->retrieveNode(1));
    diskANN->deleteNode(diskANN->precisionLTI->retrieveNode(5));
    diskANN->deleteNode(diskANN->precisionLTI->retrieveNode(10));

    //Revisames el DeleteList
    std::cout<<"DeleteList: "<<std::endl;
    for(auto node : diskANN->deleteList){
        std::cout<<node->id<<std::endl;
    }

    std::cout<<"============================================="<<std::endl;
    std::cout<< "Ahora procedemos con el Streaming Merge" << std::endl;
    std::cout<<"============================================="<<std::endl;

    // Llama a la función deletePhase
    diskANN->streamingMerge();

    return 0;
}