//
// Created by Juan Pedro on 22/11/2024.
//

#ifndef RWTEMPINDEX_H
#define RWTEMPINDEX_H
#include <unordered_map>
#include <memory>
#include "../GraphNode.h"

class RO_TempIndex {
public:
    std::unordered_map<int, std::shared_ptr<GraphNode>> nodes; // Read-Only temporary index
};


#endif //RWTEMPINDEX_H
