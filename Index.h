#ifndef INDEX_H
#define INDEX_H

#include <cstddef>
#pragma once
#include "GraphNode.h"
#include <vector>
#include <utility>
#include <unordered_set>
#include <algorithm>

class Index {
public:
    std::pair<std::vector<GraphNode>, std::vector<GraphNode>> greedySearch(const GraphNode &s, const GraphNode &xq, size_t k, size_t searchListSize);
    double distance(const GraphNode &node, const GraphNode &xq);
    void robustPrune(const GraphNode &p, std::vector<GraphNode*> &v, double alpha, size_t outDegreeBound);
    void deleteNodes(const std::unordered_set<int>& deleteList, double alpha, size_t outDegreeBound);
private:
    std::unordered_map<int, GraphNode*> graphNodes;
    std::unordered_set<int> deleteList;
};


#endif //INDEX_H
