

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
    int id;
    std::vector<uint8_t> compressedFeatures; // Compressed data (Product Quantization: 25-32 bytes)
};

class CompressedLTI {
public:
    std::unordered_map<int, std::shared_ptr<CompressedGraphNode>> compressedGraphNodes;// Compressed points

    void loadDatasetCompressed( std::string csvPath); // Load dataset (TEST)

    // Método para preparar datos de entrenamiento
    std::vector< std::pair<double,std::vector<double>>> prepareTrainingData(const std::vector<std::shared_ptr<GraphNode>>& dataset);

    // Metodo para dividir un vector en m vectores
    std::vector< std::pair<double,std::vector<std::vector<double>>>> splitVectors(const std::vector< std::pair<double,std::vector<double>>>& data, size_t m);

    // Centroides entrenados para cada subvector
    std::vector<std::vector<std::vector<double>>> centroidsPerSubvector;

    std::vector<std::vector<size_t>> trainCentroidsPQ(
        const std::vector<std::pair<double, std::vector<std::vector<double>>>>& splitData,
        size_t k);

    std::vector<size_t> encodeVector(const std::vector<double>& vector); // Método para codificar un vector
    void storeCompressedGraphNodes(const std::vector<std::pair<int, std::vector<size_t>>>& encodedData); // Almacenar nodos comprimidos
    std::vector<int> knnSearch(const std::vector<unsigned long>& query, const std::vector<std::vector<unsigned long>>& encodedDataset, int kNearestNeighbors);
};
#endif //COMPRESSEDLTI_H