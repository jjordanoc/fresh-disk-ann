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
#include <iostream>
#include <unordered_set>

double calculateReconstructionError(const std::vector<double>& original,
                                    const std::vector<double>& reconstructed) {
    if (original.size() != reconstructed.size()) {
        throw std::runtime_error("Vectors must be of the same size for error calculation");
    }
    double error = 0.0;
    for (size_t i = 0; i < original.size(); ++i) {
        error += std::pow(original[i] - reconstructed[i], 2);
    }
    return std::sqrt(error / original.size()); // RMS error
}

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

    if (trainingData.size() < 10) { // FAISS recomienda al menos 10x m
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
    size_t m = std::min(maxBytes, d); // Asegurarse de que m <= d
    size_t nbits = 8; //Numero de bits por subvector

    // *** PREPARAR DATOS DE ENTRENAMIENTO ***
    std::vector<std::vector<double>> trainingData = prepareTrainingData(node);

    size_t nx = trainingData.size(); // Número de puntos de entrenamiento disponibles
    if (nx == 0)
        throw std::runtime_error("Error: No training data available.");

    // *** AJUSTAR PARÁMETROS DINÁMICAMENTE ***
    size_t k = (1 << nbits) * m; // Número inicial de clústeres
    if (nx < k) {
        k = nx / 20; // Ajustar k a una décima parte del tamaño de los datos disponibles
        if (k < 1) {
            k = 1; // Asegurar que haya al menos un clúster
        }

        if (k < m) {
            m = k; // Ajustar m si el número de clústeres es menor que m
        }

        nbits = static_cast<size_t>(std::floor(std::log2(static_cast<double>(k) / m))); // Recalcular nbits
        if (nbits < 1) {
            nbits = 1; // Asegurar que al menos un bit por subvector
        }
    }

    faiss::IndexPQ index(d, m, nbits); //Se crea un objeto de la clase IndexPQ de Faiss
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

    // *** CALCULAR EL ERROR DE RECONSTRUCCIÓN ***
    std::vector<float> decompressedData(d); // Vector para las características descomprimidas
    index.sa_decode(1, compressedData.data(), decompressedData.data()); // Decodificar datos comprimidos

    // Calcular el error de reconstrucción
    double error = calculateReconstructionError(node->features,
                          std::vector<double>(decompressedData.begin(), decompressedData.end()));
    std::cout << "Reconstruction error for Node " << node->id << ": " << error << std::endl;
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
