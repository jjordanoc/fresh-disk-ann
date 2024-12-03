#ifndef INDEX_H
#define INDEX_H

#include <cstddef>
#include <set>
#pragma once
#include "GraphNode.h"
#include <vector>
#include <utility>
#include <unordered_set>
#include <algorithm>




class FreshVamanaIndex {
    constexpr static const double DEFAULT_ALPHA = 1.2;
    constexpr static const size_t DEFAULT_OUT_DEGREE_BOUND = 10;
    constexpr static const size_t DEFAULT_SEARCH_LIST_SIZE = 10;
    constexpr static const double DEFAULT_DELETE_ACCUMULATION_FACTOR = 0.1;
public:
    // TODO: make private
    void insert(std::shared_ptr<GraphNode> xp, std::shared_ptr<GraphNode> s, size_t searchListSize, double alpha, size_t outDegreeBound);
    std::pair<std::vector<std::shared_ptr<GraphNode>>, std::vector<std::shared_ptr<GraphNode>>> greedySearch(std::shared_ptr<GraphNode> s, std::shared_ptr<GraphNode> xq, size_t k, size_t searchListSize);
    void robustPrune(std::shared_ptr<GraphNode> p, std::vector<std::shared_ptr<GraphNode>> &v, double alpha, size_t outDegreeBound);

    // methods
    FreshVamanaIndex(const double alpha = DEFAULT_ALPHA, const size_t outDegreeBound = DEFAULT_OUT_DEGREE_BOUND, const double deleteAccumulationFactor = DEFAULT_DELETE_ACCUMULATION_FACTOR) : alpha(alpha), outDegreeBound(outDegreeBound), deleteAccumulationFactor(deleteAccumulationFactor) {}
//    FreshVamanaIndex() : alpha(DEFAULT_ALPHA), outDegreeBound(DEFAULT_OUT_DEGREE_BOUND) {}
    void deleteConsolidation();
    void deleteNode(std::shared_ptr<GraphNode> xp);
    void insert(std::shared_ptr<GraphNode> xp, size_t searchListSize = DEFAULT_SEARCH_LIST_SIZE, bool chooseRandom = false);
    std::vector<std::shared_ptr<GraphNode>> knnSearch(std::shared_ptr<GraphNode> query, size_t k, size_t searchListSize = DEFAULT_SEARCH_LIST_SIZE, bool chooseRandom = false);
    std::shared_ptr<GraphNode> getNode(size_t id);
    double distance(std::shared_ptr<GraphNode>node, std::shared_ptr<GraphNode>xq);

    std::vector<std::shared_ptr<GraphNode>> graph; //TODO: Cambiar a privado de nuevo

    void printGraph(); // DEBUG

private:
//    std::unordered_map<int, std::shared_ptr<GraphNode>> graphNodes;
//    std::unordered_set<int> deleteList;
    const double alpha;
    const size_t outDegreeBound;
    const double deleteAccumulationFactor;

//    std::unordered_map<size_t, std::shared_ptr<GraphNode>> graph;
    std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> deleteList;
};


#endif //INDEX_H