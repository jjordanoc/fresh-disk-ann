//
// Created by Juan Pedro on 22/11/2024.
//

#ifndef PRECISIONLTI_H
#define PRECISIONLTI_H

#include <vector>
#include <unordered_map>
#include <string>
#include "../GraphNode.h"

class PrecisionLTI {
private:
    std::string filePath;
    size_t outDegreeBound;
public:
    PrecisionLTI(const std::string &filePath, size_t outDegreeBound)
        : filePath(filePath), outDegreeBound(outDegreeBound) {}


    void storeNode( std::shared_ptr<GraphNode> node);
    std::shared_ptr<GraphNode> retrieveNode(size_t nodeId);

    void loadDatasetAndStoreNodes(std::string csvPath, PrecisionLTI precisionLTI); // Load CSV and Store (Test)


};
#endif //PRECISIONLTI_H
