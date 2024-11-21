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
    void insert(std::shared_ptr<GraphNode>xp, std::shared_ptr<GraphNode> s, size_t searchListSize, double alpha, size_t outDegreeBound);
    std::pair<std::vector<std::shared_ptr<GraphNode>>, std::vector<std::shared_ptr<GraphNode>>> greedySearch(std::shared_ptr<GraphNode> s, std::shared_ptr<GraphNode> xq, size_t k, size_t searchListSize);
    double distance(std::shared_ptr<GraphNode>node, std::shared_ptr<GraphNode>xq);
    void robustPrune(std::shared_ptr<GraphNode> p, std::vector<std::shared_ptr<GraphNode>> &v, double alpha, size_t outDegreeBound);
    void deleteNodes(const std::unordered_set<int>& deleteList, double alpha, size_t outDegreeBound);
private:
    std::unordered_map<int, std::shared_ptr<GraphNode>> graphNodes;
    std::unordered_set<int> deleteList;
};


#endif //INDEX_H
