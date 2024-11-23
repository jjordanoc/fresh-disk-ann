//
// Created by Juan Pedro on 22/11/2024.
//

#include "CompressedLTI.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>

void CompressedLTI::productQuantization(std::shared_ptr<GraphNode> node, size_t maxBytes) {
    std::shared_ptr<CompressedGraphNode> compressedNode = std::make_shared<CompressedGraphNode>(); //Se crea un nuevo objeto CompressedGraphNode y se asigna a un std::shared_ptr.
    compressedNode->id = node->id; //Asignar el ID del nodo original al nodo comprimido

    //Simular la compresión generando bytes aleatorios (//todo: ESTO SE DEBERIA CAMBIAR, YA QUE NO ESTAMOS UTILIZANDO PRODUCT QUANTIZATION)
    std::vector<uint8_t> compressedData(maxBytes);
    std::generate(compressedData.begin(), compressedData.end(), std::rand);

    compressedNode->compressedFeatures = compressedData; //Asignar los datos comprimidos al nodo comprimido
    compressedGraphNodes[node->id] = compressedNode; //Guardar el nodo comprimido en el mapa de nodos comprimidos
}


void CompressedLTI::loadDatasetCompressed(std::string csvPath, size_t maxBytes) {
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
            productQuantization(node, maxBytes);
            id++;
        }
    }
}