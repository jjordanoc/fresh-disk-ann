#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#pragma once
#include <vector>

class GraphNode {
public:
    int id;
    std::vector<std::shared_ptr<GraphNode>> inNeighbors;
    std::vector<std::shared_ptr<GraphNode>> outNeighbors;
    std::vector<double> features;
    bool deleted;

    void setUnionOutNeighbor(std::shared_ptr<GraphNode> newNode);

    GraphNode(int id, const std::vector<double>& features) : id(id), features(features) {}

    bool operator<(const GraphNode& rhs) const;

    struct SharedPtrComp {
        bool operator()(std::shared_ptr<GraphNode> a, std::shared_ptr<GraphNode> b) const {
            return a->id < b->id;
        }
    };
};


#endif //GRAPHNODE_H
