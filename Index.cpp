#include "Index.h"
#include <cmath>
#include <unordered_set>
#include <algorithm>

double Index::distance(const GraphNode &node, const GraphNode &xq) {
    double sum = 0.0;
    for (size_t i = 0; i < node.features.size(); ++i) {
        sum += std::pow(node.features[i] - xq.features[i], 2);
    }
    return std::sqrt(sum);
}

/*
std::pair<std::vector<GraphNode>, std::vector<GraphNode>> Index::greedySearch(const GraphNode &s, const Vector &xq, size_t k, size_t searchListSize) {
    s: start node, xq: query vector, k: number of neighbors to find, searchListSize: size of the search list
    ι <- {s} //Lista de nodos expandidos
    𝑉 <- ∅ //Lista de nodos candidatos
    while ι \ 𝑉 != ∅ do: // Mientras haya nodos en ι que no esten en 𝑉
        let p* <- argminp∈(ι\𝑉){d(p, xq)} // p* es el nodo en ι (que no esten en 𝑉) que esta mas cerca de xq
        ι <- ι ∪ Nout{p*} //Los vecinos salientes de p* se agregan a ι, es decir se expandiran (osea se visitaran)
        𝑉 <- 𝑉 ∪ {p*}  //Se marca p* como candidato a vecino mas cercano
        if |ι| > L then: //Si el size de ι supera el limite de la search list
            update ι to retain closest L nodes to xq //se recorta ι para que solo contenga los L nodos mas cercanos a xq
    return [closest k nodes in 𝑉], [all nodes in 𝑉] //Se retornan los k nodos mas cercanos a xq y todos los nodos candidatos
}
*/
std::pair<std::vector<GraphNode>, std::vector<GraphNode>>
Index::greedySearch(const GraphNode &s, const GraphNode &xq, size_t k, size_t searchListSize) {
    //ι <- {s}
    std::vector<const GraphNode*> expandedList = {&s};

    //𝑉 <- ∅
    std::vector<const GraphNode*> candidateList;

    //while ι \ 𝑉 != ∅ do:
    while (true) {
        // Encontrar todos los nodos en ι que no están en 𝑉
        std::vector<const GraphNode*> difference;
        for (const auto* node : expandedList) {
            if (std::find(candidateList.begin(), candidateList.end(), node) == candidateList.end()) {
                difference.push_back(node);
            }
        }

        //Si no hay diferencia, terminar
        if (difference.empty()) {
            break;
        }

        //let p* <- argminp∈(ι\𝑉){d(p, xq)}
        auto minIt = std::min_element(difference.begin(), difference.end(),
            [&](const GraphNode* a, const GraphNode* b) {
                return distance(*a, xq) < distance(*b, xq);
            });
        const GraphNode* pStar = *minIt;

        //ι <- ι ∪ Nout{p*}
        for (const auto* neighbor : pStar->outNeighbors) {
            if (std::find(expandedList.begin(), expandedList.end(), neighbor) == expandedList.end()) {
                expandedList.push_back(neighbor);
            }
        }

        //𝑉 <- 𝑉 ∪ {p*}
        candidateList.push_back(pStar);

        //if |ι| > L then:
        if (expandedList.size() > searchListSize) {
            // update ι to retain closest L nodes to xq
            std::partial_sort(expandedList.begin(),
                            expandedList.begin() + searchListSize,
                            expandedList.end(),
                            [&](const GraphNode* a, const GraphNode* b) {
                                return distance(*a, xq) < distance(*b, xq);
                            });
            expandedList.resize(searchListSize);
        }
    }

    //Convertir los punteros a nodos
    std::vector<GraphNode> allCandidateNodes;
    for (const auto* node : candidateList) {
        allCandidateNodes.push_back(*node);
    }

    //Ordenar los candidatos por distancia
    std::sort(allCandidateNodes.begin(), allCandidateNodes.end(),
              [&](const GraphNode& a, const GraphNode& b) {
                  return distance(a, xq) < distance(b, xq);
              });

    //[closest k nodes in 𝑉]
    std::vector<GraphNode> closestKNodes;
    size_t numNodes = std::min(k, allCandidateNodes.size());
    closestKNodes.assign(allCandidateNodes.begin(),
                        allCandidateNodes.begin() + numNodes);

    //return [closest k nodes in 𝑉], [all nodes in 𝑉]
    return {closestKNodes, allCandidateNodes};
}

/*
void Index::robustPrune(const GraphNode &p, Vector &v, double alpha, size_t outDegreeBound) {
    v ← (v ∪ 𝑁out(𝑝)) \ {𝑝} // Agregamos a v (candidatos) los vecinos de p, sin incluir a p
    𝑁out(𝑝) <- ∅ // Vaciamos los vecinos de p
    while v != ∅: //Mientras siga habiendo candidatos
        p* <- argminp'∈𝑣{d(p, 𝑝')} // p* es el nodo ∈𝑣 mas cercano a p. Es distancia euclidiana
        𝑁out(𝑝) <- 𝑁out(𝑝) ∪ {p*} // Ahora p* es vecino de p
        if |𝑁out(𝑝)| =  outDegreeBound: break // Si ya alcanzamos el limite de vecinos, terminamos
        for p' ∈ 𝑣: // Para cada nodo en v
            if d(p*, p') ≤ d(p,p')/alpha then remove p' from v // Si la distancia de p* a p' es menor o igual a la distancia de p a p' dividido alpha, entonces eliminamos p' de v (Propiedad alpha-RNG)
}
*/

void Index::robustPrune(const GraphNode &p, std::vector<GraphNode*> &v, double alpha, size_t outDegreeBound) {
    //v ← (v ∪ 𝑁out(𝑝)) \ {𝑝}
    for (auto* neighbor : p.outNeighbors) {
        if (std::find(v.begin(), v.end(), neighbor) == v.end()) {
            v.push_back(neighbor);
        }
    }
    v.erase(std::remove(v.begin(), v.end(), &p), v.end());

    //𝑁out(𝑝) <- ∅
    const_cast<GraphNode&>(p).outNeighbors.clear();

    //while v != ∅:
    while (!v.empty()) {
        //p* <- argminp'∈𝑣{d(p, 𝑝')}
        auto minIt = std::min_element(v.begin(), v.end(),
            [&](const GraphNode* a, const GraphNode* b) {
                return distance(p, *a) < distance(p, *b);
            });
        GraphNode* pStar = *minIt;

        //𝑁out(𝑝) <- 𝑁out(𝑝) ∪ {p*}
        const_cast<GraphNode&>(p).outNeighbors.push_back(pStar);

        //if |𝑁out(𝑝)| = outDegreeBound: break
        if (p.outNeighbors.size() == outDegreeBound) {
            break;
        }

        //for p' ∈ 𝑣:
        for (auto it = v.begin(); it != v.end();) {
            GraphNode* pPrime = *it;
            //if d(p*, p') ≤ d(p,p')/alpha then remove p' from v
            if (distance(*pStar, *pPrime) <= distance(p, *pPrime) / alpha) {
                it = v.erase(it);
            } else {
                ++it;
            }
        }
    }
}
