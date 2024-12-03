#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#pragma once

#include <vector>
#include <memory>
#include <set>

class GraphNode {
public:
    struct SharedPtrComp {
        bool operator()(std::shared_ptr<GraphNode> a, std::shared_ptr<GraphNode> b) const {
            return a->id < b->id;
        }
    };

    int id;
    std::set<std::shared_ptr<GraphNode>, SharedPtrComp> outNeighbors;
    std::vector<double> features;
    bool deleted;

    GraphNode(int id, const std::vector<double> &features) : id(id), features(features) {}

    bool operator<(const GraphNode &rhs) const;


};


#endif //GRAPHNODE_H