//
// Created by Joaquin on 14/11/24.
//

#ifndef FRESHDISKANN_INDEX_H
#define FRESHDISKANN_INDEX_H

#include <vector>
#include "GraphNode.h"
#include "Vector.h"


class Index {
    std::pair<std::vector<GraphNode>, std::vector<GraphNode>> greedySearch(const GraphNode &s, const Vector &xq, size_t k, size_t searchListSize);

};


#endif //FRESHDISKANN_INDEX_H
