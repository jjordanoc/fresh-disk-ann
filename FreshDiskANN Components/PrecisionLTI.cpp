//
// Created by Juan Pedro on 22/11/2024.
//
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

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



//Carga un conjunto de nodos desde un archivo CSV y los almacena en el PrecisionLTI.
void PrecisionLTI::loadDatasetAndStoreNodes(std::string csvPath, PrecisionLTI precisionLTI) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + csvPath);
    }

    size_t id = 1;
    std::string line;
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
            precisionLTI.storeNode(node);
            std::cout << "Storing node with ID: " << id << ", Features: ";
            for (const auto &feature : features) {
                std::cout << feature << " ";
            }
            std::cout << std::endl;
            id++;
        }
    }
}


//Carga los 5 primeros nodos desde un archivo CSV, los une entre sí y los almacena en el PrecisionLTI.
void PrecisionLTI::loadDatasetAndcreateConectionsAndStoreNodes(std::string csvPath, PrecisionLTI precisionLTI) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + csvPath);
    }

    size_t id = 1;
    std::string line;
    std::vector<std::shared_ptr<GraphNode>> nodes;

    // Cargar los 5 primeros nodos
    while (std::getline(file, line) && id <= 5) {
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

    // Unir los nodos entre sí
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < nodes.size(); ++j) {
            if (i != j) {
                nodes[i]->outNeighbors.push_back(nodes[j]);
            }
        }
    }

    // Guardar los nodos y mostrar sus conexiones
    for (const auto &node : nodes) {
        precisionLTI.storeNode(node);
        std::cout << "Storing node with ID: " << node->id << ", Connected to: ";
        for (const auto &neighbor : node->outNeighbors) {
            std::cout << neighbor->id << " ";
        }
        std::cout << std::endl;
    }
}