//
// Created by Juan Pedro on 22/11/2024.
//

#include "CompressedLTI.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>

void CompressedLTI::PQandStoreNode(std::shared_ptr<GraphNode> node, size_t maxBytes) {
    std::shared_ptr<CompressedGraphNode> compressedNode = productQuantization(node, maxBytes);
    compressedGraphNodes[node->id] = compressedNode;
}

std::shared_ptr<CompressedGraphNode> CompressedLTI::productQuantization(std::shared_ptr<GraphNode> node, size_t maxBytes) {
    std::shared_ptr<CompressedGraphNode> compressedNode = std::make_shared<CompressedGraphNode>();
    compressedNode->id = node->id;

    //Compress by truncating the features to maxBytes
    std::vector<uint8_t> compressedData(node->features.begin(), node->features.begin() + std::min(maxBytes, node->features.size()));

    //Save the compressed data to the compressed node
    compressedNode->compressedFeatures = compressedData;

    return compressedNode;
}

//(TEST)
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