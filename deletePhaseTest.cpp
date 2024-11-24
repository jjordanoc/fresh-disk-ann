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

void readAndPrintPrecisionLTIs(const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Failed to open file for reading");
    }

    while (inFile) {
        // Read the node ID
        int id;
        inFile.read(reinterpret_cast<char*>(&id), sizeof(id));
        if (inFile.eof()) break;

        // Read the size of the features vector
        size_t featureSize;
        inFile.read(reinterpret_cast<char*>(&featureSize), sizeof(featureSize));

        // Read the features
        std::vector<double> features(featureSize);
        inFile.read(reinterpret_cast<char*>(features.data()), featureSize * sizeof(double));

        // Read the number of out-neighbors
        size_t neighborCount;
        inFile.read(reinterpret_cast<char*>(&neighborCount), sizeof(neighborCount));

        // Read the out-neighbors' IDs
        std::vector<int> outNeighbors(neighborCount);
        for (size_t i = 0; i < neighborCount; ++i) {
            inFile.read(reinterpret_cast<char*>(&outNeighbors[i]), sizeof(outNeighbors[i]));
        }

        // Skip the padding
        size_t currentSize = sizeof(id) + sizeof(featureSize) + featureSize * sizeof(double) +
                             sizeof(neighborCount) + neighborCount * sizeof(int);
        size_t paddingSize = 4096 - currentSize;
        inFile.seekg(paddingSize, std::ios::cur);

        // Print the node details
        std::cout << "Node ID: " << id << " | ";
        /*
        std::cout << "Features: ";
        for (const auto& feature : features) {
            std::cout << feature << " ";
        }
        */
        std::cout << " | Out-Neighbors: ";
        for (const auto& neighborId : outNeighbors) {
            std::cout << neighborId << " ";
        }
        std::cout << std::endl;
    }

    inFile.close();
}

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

            for (const auto &neighborId : neighborIds) {
                if (neighborId != node->id) {
                    std::shared_ptr<GraphNode> neighbor = std::make_shared<GraphNode>(neighborId, std::vector<double>{1});
                    node->outNeighbors.push_back(neighbor);
                }
            }

            diskANN->insert(node);

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
    loadDatasetAndStoreNodes("C:/Users/Juan Pedro/Desktop/siftsmall_base.csv", 10, diskANN, 20);

    std::cout<<"============================================="<<std::endl;
    std::cout<< "Resultados despues de la insercion" << std::endl;
    std::cout<<"============================================="<<std::endl;

    //Leemos el precisionLTI
    std::cout<<"============================================="<<std::endl;
    std::cout<<"PrecisionLTI: "<<std::endl;
    readAndPrintPrecisionLTIs("precisionLTI.dat");

    //Leemos el rwTempIndex
    std::cout<<"============================================="<<std::endl;
    std::cout<<"rwTempIndex: "<<std::endl;
    for (const auto& node : diskANN->rwTempIndex->graph) {
        std::cout << "Node ID: " << node->id << " | ";

        /*
        std::cout << "Features: ";
        for (const auto& feature : node->features) {
            std::cout << feature << " ";
        }
        std::cout << std::endl;
        */
        std::cout << "Out-Neighbors: ";
        for (const auto& neighbor : node->outNeighbors) {
            std::cout << neighbor->id << " ";
        }
        std::cout << std::endl;
    }

    //leemos el compressedLTI
    std::cout<<"============================================="<<std::endl;
    std::cout<<"compressedLTI: "<<std::endl;
    for (const auto& [id, compressedNode] : diskANN->compressedLTI.compressedGraphNodes) {
        std::cout << "Node ID: " << compressedNode->id << " | Compressed Features: ";
        for (const auto& feature : compressedNode->compressedFeatures) {
            std::cout << static_cast<int>(feature) << " ";
        }
        std::cout << std::endl;
    }


    std::cout<<"============================================="<<std::endl;
    std::cout<< "Ahora procedemos con la eliminacion de nodos" << std::endl;
    std::cout<<"============================================="<<std::endl;

    //Eliminamos un nodo
    diskANN->deleteNode(diskANN->precisionLTI->retrieveNode(1));
    diskANN->deleteNode(diskANN->precisionLTI->retrieveNode(20));
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

    std::cout<<"============================================="<<std::endl;
    std::cout<< "Resultados:" << std::endl;
    std::cout<<"============================================="<<std::endl;


    //Revisamos el DeleteList
    std::cout<<"DeleteList despues del StreamingMerge: "<<std::endl;
    for(auto node : diskANN->deleteList){
        std::cout<<node->id<<std::endl;
    }

    //Leemos el rwTempIndex
    std::cout<<"============================================="<<std::endl;
    std::cout<<"============================================="<<std::endl;
    std::cout<<"rwTempIndex: "<<std::endl;
    for (const auto& node : diskANN->rwTempIndex->graph) {
        std::cout << "Node ID: " << node->id << " | ";
        /*
        std::cout << "Features: ";
        for (const auto& feature : node->features) {
            std::cout << feature << " ";
        }
        std::cout << std::endl;
        */
        std::cout << "Out-Neighbors: ";
        for (const auto& neighbor : node->outNeighbors) {
            std::cout << neighbor->id << " ";
        }
        std::cout << std::endl;
    }


    //Leemos el precisionLTI
    std::cout<<"============================================="<<std::endl;
    std::cout<<"============================================="<<std::endl;
    std::cout<<"PrecisionLTI: "<<std::endl;
    readAndPrintPrecisionLTIs("precisionLTI.dat");

    //Leemos el intermediateLTI
    std::cout<<"============================================="<<std::endl;
    std::cout<<"============================================="<<std::endl;
    std::cout<<"IntermediateLTI: "<<std::endl;
    readAndPrintPrecisionLTIs("intermediateLTI.dat");

    return 0;
}