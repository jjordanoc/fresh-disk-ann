

//
// Created by Juan Pedro on 22/11/2024.
//

#ifndef COMPRESSEDLTI_H
#define COMPRESSEDLTI_H
#include <unordered_map>
#include <memory>
#include <string>
#include <faiss/IndexPQ.h>
#include <faiss/AutoTune.h>
#include <faiss/IndexFlat.h>
#include <faiss/IndexPreTransform.h>
#include <faiss/VectorTransform.h>
#include <vector>
#include "../GraphNode.h"

class CompressedGraphNode {
public:
    int id;                              // ID único del nodo
    std::vector<uint8_t> compressedFeatures; // Compressed data (Product Quantization: 25-32 bytes)
};

class CompressedLTI {
public:
    // Mapa de nodos comprimidos
    std::unordered_map<int, std::shared_ptr<CompressedGraphNode>> compressedGraphNodes;

    // Cargar índice desde SSD
    void loadFromSSD(const std::string& filePath);

    // Guardar índice en SSD
    void saveToSSD(const std::string& filePath);

    // Construcción del índice comprimido
    void buildCompressedIndex(const std::vector<std::shared_ptr<GraphNode>>& dataset);

    std::vector<uint8_t> compressFeatures(const std::vector<double>& features);

    // Calcular la distancia entre dos nodos
    float distance(const std::vector<float>& query, int nodeId);

    // Búsqueda k-NN en el índice comprimido
    std::vector<int> knnSearch(const std::vector<float>& query, int k);

    // Agregar nodos (almacenados temporalmente en TempIndex, no aplicados en tiempo real)
    void addNode(int id, const std::vector<double>& data);

    // Marcar nodos para eliminación
    void markForDeletion(int id);

    // Destructor
    ~CompressedLTI() {
        delete pqIndex;
    }
private:
    faiss::IndexPQ* pqIndex;
    std::vector<int> deleteList; // Lista de nodos marcados para eliminación
    std::shared_ptr<CompressedGraphNode> compressGraphNode(const GraphNode& node, CompressedGraphNode& compressedNode); // Comprimir nodo
};
#endif //COMPRESSEDLTI_H
