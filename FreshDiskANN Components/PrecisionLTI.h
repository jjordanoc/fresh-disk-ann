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
    std::shared_ptr<size_t> nodeCount; //To store the number of nodes


public:
    PrecisionLTI(std::string filePath, size_t outDegreeBound)
        : filePath(filePath), outDegreeBound(outDegreeBound), nodeCount(std::make_shared<size_t>(0)) {}


    void storeNode( std::shared_ptr<GraphNode> node);
    std::shared_ptr<GraphNode> retrieveNode(size_t nodeId);

    std::shared_ptr<size_t> getNodeCount() const { return nodeCount; } // Getter for nodeCount


    void loadDatasetAndStoreNodes(std::string csvPath, size_t maxNeighbours);

};
#endif //PRECISIONLTI_H
