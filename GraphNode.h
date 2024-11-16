//
// Created by Joaquin on 14/11/24.
//

#ifndef FRESHDISKANN_GRAPHNODE_H
#define FRESHDISKANN_GRAPHNODE_H

#include <vector>

class GraphNode {
    size_t id;
    double distance;
    bool expanded;
public:
    GraphNode(unsigned id, float distance) : id{id}, distance{distance}, expanded(false) {
    }

    inline bool operator<(const GraphNode &other) const {
        return distance < other.distance || (distance == other.distance && id < other.id);
    }

    inline bool operator==(const GraphNode &other) const {
        return (id == other.id);
    }
};


#endif //FRESHDISKANN_GRAPHNODE_H
