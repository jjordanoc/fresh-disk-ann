//
// Created by Joaquin on 15/11/24.
//

#include "Graph.h"

void Graph::addNeighbor(const size_t pos, size_t neighborId) {
    _graph[pos].emplace_back(neighborId);
    if (_max_observed_degree < _graph[pos].size())
    {
        _max_observed_degree = _graph[pos].size();
    }
}

const std::vector<size_t> &Graph::getNeighbors(const size_t pos) const {
    return _graph.at(pos);
}
