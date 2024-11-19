#ifndef INDEX_H
#define INDEX_H

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
private:

};


#endif //INDEX_H
