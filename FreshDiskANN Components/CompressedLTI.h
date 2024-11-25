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
    void productQuantization(std::shared_ptr<GraphNode> node, size_t maxBytes); // Compress data (Product Quantization: 25-32 bytes)

    void loadDatasetCompressed( std::string csvPath, size_t maxBytes); // Load dataset (TEST)

    std::vector<std::vector<double> >prepareTrainingData(std::shared_ptr<GraphNode> node); // Prepare training data (TEST)




};
#endif //COMPRESSEDLTI_H
