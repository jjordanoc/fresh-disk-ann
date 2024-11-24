#include "GraphNode.h"

void GraphNode::setUnionOutNeighbor(std::shared_ptr<GraphNode> newNode) {
    if (std::find_if(outNeighbors.begin(), outNeighbors.end(), [&newNode](std::shared_ptr<GraphNode> expanded) {
        return expanded->id == newNode->id;
    }) == outNeighbors.end()) {
        outNeighbors.push_back(newNode);
    }
}

bool GraphNode::operator<(const GraphNode &rhs) const {
    return id < rhs.id;
}
