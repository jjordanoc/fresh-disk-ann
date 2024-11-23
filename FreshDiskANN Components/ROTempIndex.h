//
// Created by Juan Pedro on 22/11/2024.
//

#ifndef ROTEMPINDEX_H
#define ROTEMPINDEX_H
#include <unordered_map>
#include <memory>
#include "../GraphNode.h"

class RW_TempIndex {
public:
    std::unordered_map<int, std::shared_ptr<GraphNode>> nodes; // Read-Write temporary index
};
#endif //ROTEMPINDEX_H
