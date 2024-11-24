//
// Created by Juan Pedro on 22/11/2024.
//

#ifndef COMPRESSEDLTI_H
#define COMPRESSEDLTI_H
#include <unordered_map>
#include <memory>
#include <string>
#include "../GraphNode.h"

class CompressedGraphNode {
public:
    int id;
    std::vector<uint8_t> compressedFeatures; // Compressed data (Product Quantization: 25-32 bytes)
};

class CompressedLTI {
public:
    std::unordered_map<int, std::shared_ptr<CompressedGraphNode>> compressedGraphNodes;// Compressed points

    //todo Puede que tal vez sea buena opcion que productQuantization este en FreshDiskANN.h
    std::shared_ptr<CompressedGraphNode> productQuantization(std::shared_ptr<GraphNode> node, size_t maxBytes); // Compress a node (Product Quantization: 25-32 bytes)

    void PQandStoreNode(std::shared_ptr<GraphNode> node, size_t maxBytes); // Compress a node and store it in CompressedLTI (Product Quantization: 25-32 bytes)


    void loadDatasetCompressed( std::string csvPath, size_t maxBytes); // Load dataset (TEST)




};
#endif //COMPRESSEDLTI_H
