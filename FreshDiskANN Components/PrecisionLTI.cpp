//
// Created by Juan Pedro on 22/11/2024.
//
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <set>

#include "PrecisionLTI.h"

void PrecisionLTI::storeNode(std::shared_ptr<GraphNode> node) {

    //Abre un archivo binario en modo de adición (std::ios::app). Si no se puede abrir el archivo, lanza una excepción.
    std::ofstream outFile(filePath, std::ios::binary | std::ios::app);
    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing");
    }

    //Escribe el ID del nodo en el archivo.
    outFile.write(reinterpret_cast<const char *>(&node->id), sizeof(node->id));

    //Escribe el tamaño del vector de características del nodo.
    size_t featureSize = node->features.size();
    outFile.write(reinterpret_cast<const char *>(&featureSize), sizeof(featureSize));

    //Escribe las características del nodo.
    outFile.write(reinterpret_cast<const char *>(node->features.data()), featureSize * sizeof(double));

    //Escribe el número de vecinos salientes del nodo.
    size_t neighborCount = node->outNeighbors.size();
    outFile.write(reinterpret_cast<const char *>(&neighborCount), sizeof(neighborCount));

    //Escribe los IDs de los vecinos salientes.
    for (const auto &neighbor : node->outNeighbors) {
        outFile.write(reinterpret_cast<const char *>(&neighbor->id), sizeof(neighbor->id));
    }

    //Rellena con ceros hasta completar un bloque de 4KB.
    size_t currentSize = sizeof(node->id) + sizeof(featureSize) + featureSize * sizeof(double) +
                         sizeof(neighborCount) + neighborCount * sizeof(int);
    size_t paddingSize = 4096 - currentSize;
    std::vector<char> padding(paddingSize, 0);
    outFile.write(padding.data(), paddingSize);

    outFile.close(); //Cierra el archivo.

    (*nodeCount) += 1; // Increment the node count
    //std::cout<<"Node Count: "<< *nodeCount<<std::endl;
};


std::shared_ptr<GraphNode> PrecisionLTI::retrieveNode(size_t nodeId) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Failed to open file for reading");
    }

    while (inFile) {
        // Read the node ID
        int id;
        inFile.read(reinterpret_cast<char*>(&id), sizeof(id));
        if (id == nodeId) {
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
            std::vector<std::shared_ptr<GraphNode>> outNeighbors;
            for (size_t i = 0; i < neighborCount; ++i) {
                int neighborId;
                inFile.read(reinterpret_cast<char*>(&neighborId), sizeof(neighborId));
                outNeighbors.push_back(std::make_shared<GraphNode>(neighborId, std::vector<double>()));
            }

            // Skip the padding
            size_t currentSize = sizeof(id) + sizeof(featureSize) + featureSize * sizeof(double) +
                                 sizeof(neighborCount) + neighborCount * sizeof(int);
            size_t paddingSize = 4096 - currentSize;
            inFile.seekg(paddingSize, std::ios::cur);

            // Create and return the node
            auto node = std::make_shared<GraphNode>(id, features);
            node->outNeighbors = outNeighbors;
            return node;
        } else {
            // Skip the rest of the block
            inFile.seekg(4096 - sizeof(id), std::ios::cur);
        }
    }

    throw std::runtime_error("Node not found");
}

void PrecisionLTI::loadDatasetAndStoreNodes(std::string csvPath, size_t maxNeighbours) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + csvPath);
    }

    size_t id = 1;
    std::string line;
    std::vector<std::shared_ptr<GraphNode>> nodes;

    while (std::getline(file, line)) {
        std::vector<double> features;
        std::string value;
        std::istringstream stream(line);

        while (std::getline(stream, value, ',')) {
            try {
                features.push_back(std::stod(value));
            } catch (const std::invalid_argument &) {
                throw std::runtime_error("Invalid number in file: " + value);
            }
        }

        if (!features.empty()) {
            std::shared_ptr<GraphNode> node = std::make_shared<GraphNode>(id, features);
            nodes.push_back(node);
            id++;
        }
    }

    // Assign random neighbors to each node
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);

    for (auto &node : nodes) {
        std::set<size_t> neighborIds;
        while (neighborIds.size() < maxNeighbours) {
            neighborIds.insert(dis(gen));
        }

        for (const auto &neighborId : neighborIds) {
            if (neighborId != node->id) {
                std::shared_ptr<GraphNode> neighbor = std::make_shared<GraphNode>(neighborId, std::vector<double>{});
                node->outNeighbors.push_back(neighbor);
            }
        }

        storeNode(node);

        //std::cout << "Storing node with ID: " << node->id << std::endl;
    }
}