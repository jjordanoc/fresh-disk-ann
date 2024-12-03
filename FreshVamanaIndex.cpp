#include "FreshVamanaIndex.h"
#include <cmath>
#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <random>
#include <iostream>
#include <list>
#include "Utils.hpp"


double FreshVamanaIndex::distance(std::shared_ptr<GraphNode> node, std::shared_ptr<GraphNode> xq) {
    double sum = 0.0;
    for (size_t i = 0; i < node->features.size(); ++i) {
        sum += std::pow(node->features[i] - xq->features[i], 2);
    }
    return std::sqrt(sum);
}

/*
void FreshVamanaIndex::insert(const GraphNode &xp, const GraphNode &s, size_t searchListSize, double alpha, size_t outDegreeBound) {
    v <- ∅ // Lista de nodos candidatos
    ι <- ∅ // Lista de nodos expandidos
    [ι, v] <- greedySearch(s, p, k=1, searchListSize) // Se aplica greedySearch a s y p
    set p's out neighbors to be Nout{p} = RobustPrune(s, v, alpha, outDegreeBound) // v son los nodos candidatos a ser los mas cercanos a p. Robust Prune asegura que p no tenga mas de R conexiones salientes y elimina conexiones redundantes.
    for each j ∈ Nout{p} do: // Para cada nodo en los vecinos salientes de p
        if |Nout{j} ∪ {p}| > R then: // Si j ya tiene R vecinos salientes, aplicamos RobustPrune a j nuevamente, considerando la nueva conexion con p para asegurar que j no exceda el limite de R conexiones.
            Nout(j) <- RobustPrune(j, Nout(j) ∪ {p}, alpha, outDegreeBound)
        else:
            Nout(j) <- Nout(j) ∪ {p} // Si j no excede el limite de R conexiones, simplemente agregamos la conexion con p
 */
void FreshVamanaIndex::insert(std::shared_ptr<GraphNode> xp, std::shared_ptr<GraphNode> s, size_t searchListSize,
                              double alpha, size_t outDegreeBound) {
//    // v < - ∅ // Lista de nodos candidatos
//    ι < - ∅ // Lista de nodos expandidos
    auto [candidateList, expandedList] = greedySearch(s, xp, 1, searchListSize); // Se aplica greedySearch a s y p
    // set p's out neighbors to be Nout{p} = RobustPrune(s, v, alpha, outDegreeBound) // v son los nodos candidatos a ser los mas cercanos a p. Robust Prune asegura que p no tenga mas de R conexiones salientes y elimina conexiones redundantes.
    std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> expandedSet{expandedList.begin(), expandedList.end()};
    robustPrune(xp, expandedSet, alpha, outDegreeBound);
    // for each j ∈ Nout{p} do: // Para cada nodo en los vecinos salientes de p
    for (auto outNeighbor: xp->outNeighbors) {
        //        if |Nout{j} ∪ {p}| > R then: // Si j ya tiene R vecinos salientes, aplicamos RobustPrune a j nuevamente, considerando la nueva conexion con p para asegurar que j no exceda el limite de R conexiones.
        if (outNeighbor->outNeighbors.size() + 1 > outDegreeBound) {
            //            Nout(j) <- RobustPrune(j, Nout(j) ∪ {p}, alpha, outDegreeBound)
            auto nOutCopy = outNeighbor->outNeighbors;
            nOutCopy.insert(xp);
            robustPrune(outNeighbor, nOutCopy, alpha, outDegreeBound);
        } else {
            //            Nout(j) <- Nout(j) ∪ {p} // Si j no excede el limite de R conexiones, simplemente agregamos la conexion con p
            outNeighbor->outNeighbors.insert(xp);
        }
    }

}

void FreshVamanaIndex::insert(std::shared_ptr<GraphNode> xp, size_t searchListSize, bool chooseRandom) {
    graph.insert({xp->id, xp});
    auto startingNode = graph.begin()->second;
//    if (chooseRandom) {
//        std::random_device dev;
//        std::mt19937 rng(dev());
//        std::uniform_int_distribution<std::mt19937::result_type> uniform(0, graph.size() - 1);
//        startingNode = graph[uniform(rng)];
//    }
    insert(xp, startingNode, searchListSize, alpha, outDegreeBound);
}


std::vector<std::shared_ptr<GraphNode>>
FreshVamanaIndex::knnSearch(std::shared_ptr<GraphNode> query, size_t k, size_t searchListSize, bool chooseRandom) {
    auto startingNode = graph.begin()->second;
//    if (chooseRandom) {
//        std::random_device dev;
//        std::mt19937 rng(dev());
//        std::uniform_int_distribution<std::mt19937::result_type> uniform(0, graph.size() - 1);
//        startingNode = graph[uniform(rng)];
//    }
    auto [closestK, candidateList] = greedySearch(startingNode, query, k, searchListSize);
    return closestK;
}

/*
std::pair<std::vector<GraphNode>, std::vector<GraphNode>> FreshVamanaIndex::greedySearch(const GraphNode &s, const Vector &xq, size_t k, size_t searchListSize) {
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
std::pair<std::vector<std::shared_ptr<GraphNode>>, std::vector<std::shared_ptr<GraphNode>>>
FreshVamanaIndex::greedySearch(std::shared_ptr<GraphNode> s, std::shared_ptr<GraphNode> xq, size_t k,
                               size_t searchListSize) {
    //ι <- {s}
    std::vector<std::shared_ptr<GraphNode>> expandedList = {s};

    //𝑉 <- ∅
    std::vector<std::shared_ptr<GraphNode>> candidateList;

    //while ι \ 𝑉 != ∅ do:
    while (true) {
        // Encontrar todos los nodos en ι que no están en 𝑉
        std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> difference;
//        SET_DIFFERENCE(expandedList, candidateList, difference);
        for (auto node: expandedList) {
            if (std::find_if(candidateList.begin(), candidateList.end(), [&node](std::shared_ptr<GraphNode> expanded) {
                return expanded->id == node->id;
            }) == candidateList.end()) {
                difference.insert(node);
            }
        }

        //Si no hay diferencia, terminar
        if (difference.empty()) {
            break;
        }

        //let p* <- argminp∈(ι\𝑉){d(p, xq)}
        auto minIt = std::min_element(difference.begin(), difference.end(),
                                      [&](std::shared_ptr<GraphNode> a, std::shared_ptr<GraphNode> b) {
                                          return distance(a, xq) < distance(b, xq);
                                      });
        std::shared_ptr<GraphNode> pStar = *minIt;
//        std::cout << "best candidate so far: " << pStar->id << std::endl;

        //ι <- ι ∪ Nout{p*}
        for (auto neighbor: pStar->outNeighbors) {
            if (std::find_if(expandedList.begin(), expandedList.end(),
                             [&neighbor](std::shared_ptr<GraphNode> expanded) {
                                 return expanded->id == neighbor->id;
                             }) == expandedList.end()) {
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
                              [&](std::shared_ptr<GraphNode> a, std::shared_ptr<GraphNode> b) {
                                  return distance(a, xq) < distance(b, xq);
                              });
            expandedList.resize(searchListSize);
        }
    }

    //Convertir los punteros a nodos
//    std::vector<std::shared_ptr<GraphNode>> allCandidateNodes;
//    for (auto node: candidateList) {
//        allCandidateNodes.push_back(node);
//    }

    //Ordenar los candidatos por distancia
    std::sort(candidateList.begin(), candidateList.end(),
              [&](std::shared_ptr<GraphNode> a, std::shared_ptr<GraphNode> b) {
                  return distance(a, xq) < distance(b, xq);
              });

    //[closest k nodes in 𝑉]
    std::vector<std::shared_ptr<GraphNode>> closestKNodes;
    size_t numNodes = std::min(k, candidateList.size());
    for (auto node: candidateList) {
        if (!node->deleted) {
            closestKNodes.push_back(node);
        }
        if (closestKNodes.size() == numNodes) {
            break;
        }
    }

    assert(closestKNodes.size() == numNodes);

    //return [closest k nodes in 𝑉], [all nodes in 𝑉]
    return {closestKNodes, candidateList};
}

/*
void FreshVamanaIndex::robustPrune(const GraphNode &p, Vector &v, double alpha, size_t outDegreeBound) {
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

void
FreshVamanaIndex::robustPrune(std::shared_ptr<GraphNode> p,
                              std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> &v, double alpha,
                              size_t outDegreeBound) {
    //v ← (v ∪ 𝑁out(𝑝)) \ {𝑝}
    for (auto neighbor: p->outNeighbors) {
        if (v.find(neighbor) == v.end()) {
            v.insert(neighbor);
        }
    }
    v.erase(p);

    //𝑁out(𝑝) <- ∅
    p->outNeighbors.clear();

    //while v != ∅:
    while (!v.empty()) {
        //p* <- argminp'∈𝑣{d(p, 𝑝')}
        auto minIt = std::min_element(v.begin(), v.end(),
                                      [&](std::shared_ptr<GraphNode> a, std::shared_ptr<GraphNode> b) {
                                          return distance(p, a) < distance(p, b);
                                      });
        std::shared_ptr<GraphNode> pStar = *minIt;

        //𝑁out(𝑝) <- 𝑁out(𝑝) ∪ {p*}
        // do not add deleted nodes to the list
        // TODO: verify if tihs is reasonable
        if (pStar->deleted) {
            v.erase(minIt);
            continue;
        }
//        if (!pStar->deleted) {
//            p->outNeighbors.push_back(pStar);
//        }
        p->outNeighbors.insert(pStar);

        //if |𝑁out(𝑝)| = outDegreeBound: break
        if (p->outNeighbors.size() == outDegreeBound) {
            break;
        }

        //for p' ∈ 𝑣:
        for (auto it = v.begin(); it != v.end();) {
            std::shared_ptr<GraphNode> pPrime = *it;
            //if d(p*, p') ≤ d(p,p')/alpha then remove p' from v
            if (distance(pStar, pPrime) * alpha <= distance(p, pPrime)) {
                it = v.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void FreshVamanaIndex::deleteNode(std::shared_ptr<GraphNode> xp) {
    // only mark as deleted
    xp->deleted = true;
    // add to delete list
    deleteList.insert(xp);
    // 1-10% of the index size
    if (deleteAccumulationFactor * graph.size() <= deleteList.size()) {
#ifdef DEBUG
        std::cout << "Consolidating delete." << std::endl;
#endif
        deleteConsolidation();
    }
}

void FreshVamanaIndex::deleteConsolidation() {

    std::list<std::shared_ptr<GraphNode>> nodesToPrune;
    for (const auto &[id, node]: graph) {
        // foreach p in P \ L_D (omit nodes in L_D)
        if (deleteList.find(node) != deleteList.end()) {
            continue;
        }
        // Nout(p) n L_D != {}
        std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> deletedNeighbors;
        std::set_intersection(deleteList.begin(), deleteList.end(), node->outNeighbors.begin(),
                              node->outNeighbors.end(), std::inserter(deletedNeighbors, deletedNeighbors.begin()));
//        auto deletedNeighbors = std::find_if(node->outNeighbors.begin(), node->outNeighbors.end(), [this](std::shared_ptr<GraphNode> outNeighbor){
//            return deleteList.find(outNeighbor) != deleteList.end();
//        });
        if (deletedNeighbors.empty()) {
            continue;
        }

        // "C ← 𝑁out(𝑝) \ D" Inicializamos la lista de candidatos
        std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> candidateList;
        for (auto outNeighbor: node->outNeighbors) {
            // verify neighbor not in D
            if (deletedNeighbors.find(outNeighbor) == deletedNeighbors.end()) {
                candidateList.insert(outNeighbor);
            }
        }

        // "foreach 𝑣 ∈ D do C ← C ∪ 𝑁out(𝑣)"
        for (auto deletedNeighbor: deletedNeighbors) {
            candidateList.insert(deletedNeighbor->outNeighbors.begin(), deletedNeighbor->outNeighbors.end());
        }

        // "C ← C \ D"
        std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> candidates;
        SET_DIFFERENCE(candidateList, deletedNeighbors, candidates);
        robustPrune(node, candidates, alpha, outDegreeBound);
    }
    // update graph
    for (const auto &node: deleteList) {
        graph.erase(node->id);
    }
//    auto removeIter = std::remove_if(graph.begin(), graph.end(), [this](std::shared_ptr<GraphNode> node){
//        return node->deleted;
//    });
//    graph.erase(removeIter, graph.end());
}

std::shared_ptr<GraphNode> FreshVamanaIndex::getNode(size_t id) {
//    auto result = std::find_if(graph.begin(), graph.end(), [&id](std::shared_ptr<GraphNode> node){
//        return node->id == id;
//    });
//    return *result;
    return graph[id];
}

void FreshVamanaIndex::printGraph() {
    std::cout << std::endl;
    for (const auto &[id, node]: graph) {
        std::cout << id;
        if (node->deleted) {
            std::cout << "(D)";
        }
        std::cout << "-> ";
        for (auto outNeighbor: node->outNeighbors) {
            std::cout << outNeighbor->id;
            if (outNeighbor->deleted) {
                std::cout << "(D)";
            }
            std::cout << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}