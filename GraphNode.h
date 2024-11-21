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

    GraphNode(int id, const std::vector<double>& features) : id(id), features(features) {}
};

#endif //GRAPHNODE_H
