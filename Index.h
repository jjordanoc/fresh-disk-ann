//
// Created by Joaquin on 14/11/24.
//

#ifndef FRESHDISKANN_INDEX_H
#define FRESHDISKANN_INDEX_H

#include <vector>
#include "GraphNode.h"
#include "Vector.h"
#include "Graph.h"

class Index {
    Graph _graph;
public:
    std::pair<std::vector<GraphNode>, std::vector<GraphNode>> greedySearch(const GraphNode &s, const Vector &xq, size_t k, size_t searchListSize);
    void insert(const Vector &xq, const GraphNode &s, size_t searchListSize, double alpha, size_t outDegreeBound);
};


#endif //FRESHDISKANN_INDEX_H
