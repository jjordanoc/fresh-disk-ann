//
// Created by Juan Pedro on 22/11/2024.
//

#include "CompressedLTI.h"
#include <faiss/IndexPQ.h>
#include <faiss/AutoTune.h>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <unordered_set>

std::vector<std::vector<double> > CompressedLTI::prepareTrainingData(std::shared_ptr<GraphNode> startNode){
    std::vector<std::vector<double>> trainingData;
    std::unordered_set<int> visited;
    std::queue<std::shared_ptr<GraphNode>> queue;

    queue.push(startNode);
    visited.insert(startNode->id);

    while (!queue.empty() && trainingData.size() < 300) {
        auto currentNode = queue.front();
        queue.pop();

        if (!currentNode->features.empty()) {
            trainingData.push_back(currentNode->features);
        }
        for (const auto& neighbor : currentNode->outNeighbors) {
            if (visited.find(neighbor->id) == visited.end()) {
                visited.insert(neighbor->id);
                queue.push(neighbor);
            }
        }
    }

    if (trainingData.size() < 50) { // FAISS recomienda al menos 10x m
        throw std::runtime_error("Error: Not enough training data. At least 10 samples are needed.");
    }

    return trainingData;
}

void CompressedLTI::productQuantization(std::shared_ptr<GraphNode> node, size_t maxBytes) {
    /*
    std::shared_ptr<CompressedGraphNode> compressedNode = std::make_shared<CompressedGraphNode>(); //Se crea un nuevo objeto CompressedGraphNode y se asigna a un std::shared_ptr.
    compressedNode->id = node->id; //Asignar el ID del nodo original al nodo comprimido

    //Simular la compresión generando bytes aleatorios (//todo: ESTO SE DEBERIA CAMBIAR, YA QUE NO ESTAMOS UTILIZANDO PRODUCT QUANTIZATION)
    std::vector<uint8_t> compressedData(maxBytes);
    std::generate(compressedData.begin(), compressedData.end(), std::rand);

    compressedNode->compressedFeatures = compressedData; //Asignar los datos comprimidos al nodo comprimido
    compressedGraphNodes[node->id] = compressedNode; //Guardar el nodo comprimido en el mapa de nodos comprimidos
    */
    if(node->features.empty()) throw std::runtime_error("Error: Empty features vector");

    size_t d = node->features.size(); //Dimension del vector de features
    size_t m = maxBytes;
    size_t nbits = 8; //Numero de bits por subvector

    if (d % m != 0) {
        throw std::invalid_argument("Error: The number of bytes (m) must divide the dimension (d) evenly.");
    }

    faiss::IndexPQ index(d, m, nbits); //Se crea un objeto de la clase IndexPQ de Faiss

    // *** ENTRENAMIENTO DEL ÍNDICE ***
    // Obtener datos de entrenamiento
    std::vector<std::vector<double>> trainingData = prepareTrainingData(node);

    // Convertir datos de entrenamiento a un array plano
    size_t numTrainingVectors = trainingData.size();
    std::vector<float> flatTrainingData(numTrainingVectors * d);
    for (size_t i = 0; i < numTrainingVectors; ++i) {
        std::copy(trainingData[i].begin(), trainingData[i].end(), flatTrainingData.begin() + i * d);
    }

    // Entrenar el índice con los datos de entrenamiento
    index.train(numTrainingVectors, flatTrainingData.data());

    // Comprimir las características del nodo actual
    std::vector<uint8_t> compressedData(m);
    std::vector<float> floatFeatures(node->features.begin(), node->features.end()); // Convertir a float
    index.sa_encode(1, floatFeatures.data(), compressedData.data());

    std::shared_ptr<CompressedGraphNode> compressedNode = std::make_shared<CompressedGraphNode>();
    compressedNode->id = node->id;
    compressedNode->compressedFeatures = compressedData;
    compressedGraphNodes[node->id] = compressedNode;
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
