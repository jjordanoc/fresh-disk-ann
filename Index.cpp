//
// Created by Joaquin on 14/11/24.
//

#include "Index.h"
#include <set>

void Index::insert(const Vector &xq, const GraphNode &s, size_t searchListSize, double alpha, size_t outDegreeBound) {


}

std::pair<std::vector<GraphNode>, std::vector<GraphNode>>
Index::greedySearch(const GraphNode &s, const Vector &xq, size_t k, size_t searchListSize) {
    std::set<GraphNode> approxNearestNeighbors, visitedNodes, difference;
    approxNearestNeighbors.insert(s);

    do {
        std::set_difference(approxNearestNeighbors.begin(), approxNearestNeighbors.end(), visitedNodes.begin(), visitedNodes.end(), difference.begin())
    } while ()


    return std::pair<std::vector<GraphNode>, std::vector<GraphNode>>();
}
