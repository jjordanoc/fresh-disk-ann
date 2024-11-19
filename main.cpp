// main.cpp
#include <iostream>
#include "Index.h"

void testNodeCreation() {
    GraphNode node1(1, {1.0, 2.0, 3.0});
    GraphNode node2(2, {4.0, 5.0, 6.0});
    node1.outNeighbors.push_back(&node2);
    node2.inNeighbors.push_back(&node1);

    std::cout << "Node 1 ID: " << node1.id << ", Features: ";
    for (const auto& feature : node1.features) {
        std::cout << feature << " ";
    }
    std::cout << "\nNode 2 ID: " << node2.id << ", Features: ";
    for (const auto& feature : node2.features) {
        std::cout << feature << " ";
    }
    std::cout << "\nNode 1 Out Neighbors: " << node1.outNeighbors.size() << ", Node 2 In Neighbors: " << node2.inNeighbors.size() << std::endl;
}

void testDistanceFunction() {
    GraphNode node1(1, {1.0, 2.0, 3.0});
    GraphNode node2(2, {4.0, 5.0, 6.0});
    Index index;
    double dist = index.distance(node1, node2);
    std::cout << "Distance between Node 1 and Node 2: " << dist << std::endl;
}
// main.cpp
#include <iostream>
#include "Index.h"

void testGreedySearch() {
    // Crear nodos con características específicas
    GraphNode node1(1, {1.0, 2.0, 3.0});
    GraphNode node2(2, {4.0, 5.0, 6.0});
    GraphNode node3(3, {7.0, 8.0, 9.0});
    GraphNode node4(4, {1.0, 3.0, 5.0});
    GraphNode node5(5, {2.0, 4.0, 6.0});
    GraphNode node6(6, {3.0, 5.0, 7.0});

    // Conectar los nodos
    node1.outNeighbors.push_back(&node2);
    node2.inNeighbors.push_back(&node1);

    node2.outNeighbors.push_back(&node3);
    node3.inNeighbors.push_back(&node2);

    node3.outNeighbors.push_back(&node4);
    node4.inNeighbors.push_back(&node3);

    node4.outNeighbors.push_back(&node5);
    node5.inNeighbors.push_back(&node4);

    node5.outNeighbors.push_back(&node6);
    node6.inNeighbors.push_back(&node5);

    // Crear el índice
    Index index;

    // Mostrar las distancias entre nodos conectados
    std::cout << "Distancias entre nodos conectados:" << std::endl;
    std::cout << "Distancia entre Node 1 y Node 2: " << index.distance(node1, node2) << std::endl;
    std::cout << "Distancia entre Node 2 y Node 3: " << index.distance(node2, node3) << std::endl;
    std::cout << "Distancia entre Node 3 y Node 4: " << index.distance(node3, node4) << std::endl;
    std::cout << "Distancia entre Node 4 y Node 5: " << index.distance(node4, node5) << std::endl;
    std::cout << "Distancia entre Node 5 y Node 6: " << index.distance(node5, node6) << std::endl;
    std::cout << "\nEjecutando búsqueda greedy desde nodo " << node2.id
                  << " hasta nodo " << node6.id << std::endl;

    auto result = index.greedySearch(node2, node6, 3, 3);

    std::cout << "\nNodos más cercanos encontrados:" << std::endl;
    for (const auto& node : result.first) {
        std::cout << "Nodo " << node.id << " (distancia a nodo 6: "
                  << index.distance(node, node6) << ")" << std::endl;
    }

    std::cout << "\nTodos los nodos candidatos:" << std::endl;
    for (const auto& node : result.second) {
        std::cout << "Nodo " << node.id << " (distancia a nodo 6: "
                  << index.distance(node, node6) << ")" << std::endl;
    }
}

void testRobustPrune() {
    // Crear nodos con características específicas
    GraphNode node1(1, {1.0, 2.0, 3.0});
    GraphNode node2(2, {4.0, 5.0, 6.0});
    GraphNode node3(3, {7.0, 8.0, 9.0});
    GraphNode node4(4, {1.0, 3.0, 5.0});
    GraphNode node5(5, {2.0, 4.0, 6.0});
    GraphNode node6(6, {3.0, 5.0, 7.0});

    // Conectar los nodos
    node1.outNeighbors.push_back(&node2);
    node1.outNeighbors.push_back(&node3);
    node1.outNeighbors.push_back(&node4);
    node1.outNeighbors.push_back(&node5);
    node1.outNeighbors.push_back(&node6);

    std::vector<GraphNode*> candidates = {&node2, &node3, &node4, &node5, &node6};

    // Crear el índice
    Index index;

    // Ejecutar robustPrune
    double alpha = 1.5;
    size_t outDegreeBound = 3;
    index.robustPrune(node1, candidates, alpha, outDegreeBound);

    // Mostrar los vecinos de node1 después de la poda
    std::cout << "Vecinos de Node 1 después de robustPrune:" << std::endl;
    for (const auto* neighbor : node1.outNeighbors) {
        std::cout << "Node " << neighbor->id << std::endl;
    }
}

int main() {
    std::cout << "Testing Node Creation..." << std::endl;
    testNodeCreation();

    std::cout << "\nTesting Distance Function..." << std::endl;
    testDistanceFunction();

    std::cout << "\nTesting Greedy Search..." << std::endl;
    testGreedySearch();

    std::cout << "\nTesting Robust Prune..." << std::endl;
    testRobustPrune();

    return 0;
}