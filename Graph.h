//
// Created by Joaquin on 15/11/24.
//

#ifndef FRESHDISKANN_GRAPH_H
#define FRESHDISKANN_GRAPH_H

#include <vector>

class Graph {
    std::vector<std::vector<size_t>> _graph;
    size_t _max_observed_degree = 0;
public:
    const std::vector<size_t>&getNeighbors(const size_t pos) const;
    void addNeighbor(const size_t pos, size_t neighborId);
};


#endif //FRESHDISKANN_GRAPH_H
