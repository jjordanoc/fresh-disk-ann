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

//Permite recuperar un nodo específico de un archivo binario, leyendo sus características y vecinos.
std::shared_ptr<GraphNode> PrecisionLTI::retrieveNode(size_t nodeId) {
    //Abre un archivo binario en modo de lectura.
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Failed to open file for reading");
    }

    while (inFile) { //Se inicia un bucle que continúa mientras haya datos en el archivo.
        //Se lee el ID del nodo del archivo.
        int id;
        inFile.read(reinterpret_cast<char *>(&id), sizeof(id));

        //Si el ID leído coincide con el nodeId buscado, se procede a leer los datos del nodo.
        if (id == nodeId) {
            //Se lee el tamaño del vector de características y luego se leen las características en un vector de double.
            size_t featureSize;
            inFile.read(reinterpret_cast<char *>(&featureSize), sizeof(featureSize));
            std::vector<double> features(featureSize);
            inFile.read(reinterpret_cast<char *>(features.data()), featureSize * sizeof(double));

            //Se lee el número de vecinos y luego se leen los IDs de los vecinos, creando nodos vacíos para ellos.
            size_t neighborCount;
            inFile.read(reinterpret_cast<char *>(&neighborCount), sizeof(neighborCount));
            std::vector<std::shared_ptr<GraphNode>> outNeighbors(neighborCount);
            for (size_t i = 0; i < neighborCount; ++i) {
                int neighborId;
                inFile.read(reinterpret_cast<char *>(&neighborId), sizeof(neighborId));
                outNeighbors[i] = std::make_shared<GraphNode>(neighborId, std::vector<double>{});
            }

            //Se crea y devuelve un shared_ptr al nodo con el ID y las características leídas.
            return std::make_shared<GraphNode>(id, features);
        } else {
            //Si el ID no coincide, se salta al siguiente bloque de 4096 bytes menos el tamaño del ID.
            inFile.seekg(4096 - sizeof(id), std::ios::cur);
        }
    }
    //Si se llega al final del archivo sin encontrar el nodo, se lanza una excepción.
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
            /*
            std::cout << "Storing node with ID: " << id << ", Features: ";
            for (const auto &feature : features) {
                std::cout << feature << " ";
            }
            std::cout << std::endl;
            */
            id++;
        }
    }
}