#include "GraphNode.h"

bool GraphNode::operator<(const GraphNode &rhs) const {
    return id < rhs.id;
}
