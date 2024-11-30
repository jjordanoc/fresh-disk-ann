//
// Created by Juan Pedro on 22/11/2024.
//

#include "CompressedLTI.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <queue>
#include <iostream>
#include <Eigen/Dense>
#include <unordered_set>


std::vector<float> datasetToMatrix(const std::vector<std::shared_ptr<GraphNode>>& dataset) {
    size_t n = dataset.size();
    size_t d = dataset[0]->features.size();
    std::vector<float> matrix(n * d);

    for (size_t i = 0; i < n; ++i) {
        const auto& features = dataset[i]->features;
        for (size_t j = 0; j < d; ++j) {
            matrix[i * d + j] = static_cast<float>(features[j]);
        }
    }
    return matrix;
}

std::vector<uint8_t> CompressedLTI::compressFeatures(const std::vector<double>& features) {
    std::vector<float> featuresFloat(features.begin(), features.end());

    // Vector para almacenar los datos comprimidos
    std::vector<uint8_t> compressed(pqIndex->code_size); // Obtener tamaño del código (m * log2(k))

    // Codificar una sola fila usando sa_encode
    pqIndex->sa_encode(1, featuresFloat.data(), compressed.data());

    return compressed;
}


void CompressedLTI::loadFromSSD(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string token;

        // Leer el ID del nodo
        std::getline(stream, token, ',');
        int nodeId = std::stoi(token);

        // Leer las características comprimidas
        std::vector<uint8_t> compressedFeatures;
        while (std::getline(stream, token, ',')) {
            compressedFeatures.push_back(static_cast<uint8_t>(std::stoi(token)));
        }

        if (compressedFeatures.size() < 25 || compressedFeatures.size() > 32) {
            throw std::runtime_error("Invalid compressed feature size for node ID: " + std::to_string(nodeId));
        }

        // Crear un CompressedGraphNode y agregarlo al mapa
        auto compressedNode = std::make_shared<CompressedGraphNode>();
        compressedNode->id = nodeId;
        compressedNode->compressedFeatures = std::move(compressedFeatures);

        compressedGraphNodes[nodeId] = compressedNode;
    }

    file.close();
}


void CompressedLTI::saveToSSD(const std::string& filePath)
{
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: " + filePath);
    }

    for (const auto& [nodeId, compressedNode] : compressedGraphNodes) {
        // Escribir el ID del nodo
        file << compressedNode->id;

        // Escribir las características comprimidas
        for (const auto& feature : compressedNode->compressedFeatures) {
            file << "," << static_cast<int>(feature); // Convertir uint8_t a int para escribirlo
        }

        file << "\n"; // Nueva línea al final de cada nodo
    }

    file.close();
}


void CompressedLTI::buildCompressedIndex(const std::vector<std::shared_ptr<GraphNode>>& dataset)
{
    if (dataset.empty()) {
        throw std::runtime_error("Dataset is empty. Cannot build compressed index.");
    }

    // Inicializar estructuras de compresión (por ejemplo, usar Faiss)
    size_t d = dataset[0]->features.size(); // Dimensión de las características originales
    size_t m = 64; // Número de subvectores //128 overfitea con 1 de recall
    size_t k = 8; // Número de centroides por subvector //8

    //faiss::IndexFlatL2 coarseQuantizer(d);
    pqIndex = new faiss::IndexPQ(d, m, k);
    std::vector<float> flatDataset = datasetToMatrix(dataset);
    pqIndex->train(dataset.size(), flatDataset.data());

    for (const auto& node : dataset) {
        // Comprimir características del nodo
        CompressedGraphNode compressedNode;
        compressedNode.id = node->id;
        compressedNode.compressedFeatures = compressFeatures(node->features);

        // Almacenar nodo comprimido
        compressedGraphNodes[node->id] = std::make_shared<CompressedGraphNode>(compressedNode);
    }
}

std::vector<int> CompressedLTI::knnSearch(const std::vector<float>& query, int k) {
    if (!pqIndex) {
        throw std::runtime_error("Compressed index is not initialized. Please build the index first.");
    }
    if (compressedGraphNodes.empty()) {
        throw std::runtime_error("The compressed index is empty.");
    }

    // Crear un vector para almacenar las distancias y los identificadores
    std::vector<std::pair<float, int>> distances;

    for (const auto& [id, compressedNode] : compressedGraphNodes) {
        // Decodificar el vector comprimido
        std::vector<float> decodedFeatures(query.size());
        pqIndex->sa_decode(1, compressedNode->compressedFeatures.data(), decodedFeatures.data());

        // Calcular la distancia euclidiana
        float distance = 0.0f;
        for (size_t i = 0; i < query.size(); ++i) {
            float diff = query[i] - decodedFeatures[i];
            distance += diff * diff;
        }

        // Guardar la distancia y el identificador
        distances.emplace_back(std::sqrt(distance), id);
    }

    // Ordenar las distancias (menor a mayor)
    std::sort(distances.begin(), distances.end());

    // Extraer los k identificadores más cercanos
    std::vector<int> knn;
    for (int i = 0; i < k && i < static_cast<int>(distances.size()); ++i) {
        knn.push_back(distances[i].second);
    }

    return knn;
}

float CompressedLTI::distance(const std::vector<float>& query, int nodeId) {
    if (!pqIndex) {
        throw std::runtime_error("Compressed index is not initialized. Please build the index first.");
    }
    if (compressedGraphNodes.empty()) {
        throw std::runtime_error("The compressed index is empty.");
    }

    // Decodificar el vector comprimido
    std::vector<float> decodedFeatures(query.size());
    pqIndex->sa_decode(1, compressedGraphNodes[nodeId]->compressedFeatures.data(), decodedFeatures.data());

    // Calcular la distancia euclidiana
    float distance = 0.0f;
    for (size_t i = 0; i < query.size(); ++i) {
        float diff = query[i] - decodedFeatures[i];
        distance += diff * diff;
    }

    return std::sqrt(distance);
}

// Agregar nodo a TempIndex (almacenado temporalmente, no aplicado en tiempo real)
void CompressedLTI::addNode(int id, const std::vector<double>& data) {
    // Comprimir las características del nodo
    CompressedGraphNode compressedNode;
    compressedNode.id = id;
    compressedNode.compressedFeatures = compressFeatures(data);

    // Agregarlo al índice temporal
    compressedGraphNodes[id] = std::make_shared<CompressedGraphNode>(compressedNode);

    std::cout << "Node " << id << " added temporarily." << std::endl;
}

void CompressedLTI::markForDeletion(int id) {
    deleteList.push_back(id);
    std::cout << "Node " << id << " marked for deletion." << std::endl;
}

std::shared_ptr<CompressedGraphNode> CompressedLTI::compressGraphNode(const GraphNode& node, CompressedGraphNode& compressedNode) {
    compressedNode.id = node.id;
    compressedNode.compressedFeatures = compressFeatures(node.features);

    std::cout << "Node " << node.id << " compressed." << std::endl;
    return std::make_shared<CompressedGraphNode>(compressedNode);
}
