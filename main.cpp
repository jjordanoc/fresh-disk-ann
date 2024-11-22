// main.cpp
#include <iostream>
#include <chrono>
#include <set>
#include "FreshVamanaTestUtils.hpp"
#include "FreshVamanaIndex.h"

void testNodeCreation() {
    std::vector<double> values = {1.0, 2.0, 3.0};
    std::shared_ptr<GraphNode> node1 = std::make_shared<GraphNode>(1, values);
    values = {4.0, 5.0, 6.0};
    std::shared_ptr<GraphNode> node2 = std::make_shared<GraphNode>(2, values);

    std::cout << "Node 1 ID: " << node1->id << ", Features: ";
    for (const auto &feature: node1->features) {
        std::cout << feature << " ";
    }
    std::cout << "\nNode 2 ID: " << node2->id << ", Features: ";
    for (const auto &feature: node2->features) {
        std::cout << feature << " ";
    }
    std::cout << "\nNode 1 Out Neighbors: " << node1->outNeighbors.size() << ", Node 2 In Neighbors: "
              << node2->inNeighbors.size() << std::endl;
}

void testDistanceFunction() {
    std::vector<double> values = {1.0, 2.0, 3.0};
    std::shared_ptr<GraphNode> node1 = std::make_shared<GraphNode>(1, values);
    values = {4.0, 5.0, 6.0};
    std::shared_ptr<GraphNode> node2 = std::make_shared<GraphNode>(2, values);
    FreshVamanaIndex index;
    double dist = index.distance(node1, node2);
    std::cout << "Distance between Node 1 and Node 2: " << dist << std::endl;
}

void testGreedySearch() {
    // Crear nodos con características específicas
    std::vector<double> values = {1.0, 2.0, 3.0};
    std::shared_ptr<GraphNode> node1 = std::make_shared<GraphNode>(1, values);
    values = {4.0, 5.0, 6.0};
    std::shared_ptr<GraphNode> node2 = std::make_shared<GraphNode>(2, values);
    values = {7.0, 8.0, 9.0};
    std::shared_ptr<GraphNode> node3 = std::make_shared<GraphNode>(3, values);
    values = {1.0, 3.0, 5.0};
    std::shared_ptr<GraphNode> node4 = std::make_shared<GraphNode>(4, values);
    values = {2.0, 4.0, 6.0};
    std::shared_ptr<GraphNode> node5 = std::make_shared<GraphNode>(5, values);
    values = {3.0, 5.0, 7.0};
    std::shared_ptr<GraphNode> node6 = std::make_shared<GraphNode>(6, values);

    // Conectar los nodos
    node1->outNeighbors.push_back(node2);
    node2->inNeighbors.push_back(node1);

    node2->outNeighbors.push_back(node3);
    node3->inNeighbors.push_back(node2);

    node3->outNeighbors.push_back(node4);
    node4->inNeighbors.push_back(node3);

    node4->outNeighbors.push_back(node5);
    node5->inNeighbors.push_back(node4);

    node5->outNeighbors.push_back(node6);
    node6->inNeighbors.push_back(node5);


    // Crear el índice
    FreshVamanaIndex index;

    // Mostrar las distancias entre nodos conectados
    std::cout << "Distancias entre nodos conectados:" << std::endl;
    std::cout << "Distancia entre Node 1 y Node 2: " << index.distance(node1, node2) << std::endl;
    std::cout << "Distancia entre Node 2 y Node 3: " << index.distance(node2, node3) << std::endl;
    std::cout << "Distancia entre Node 3 y Node 4: " << index.distance(node3, node4) << std::endl;
    std::cout << "Distancia entre Node 4 y Node 5: " << index.distance(node4, node5) << std::endl;
    std::cout << "Distancia entre Node 5 y Node 6: " << index.distance(node5, node6) << std::endl;
    std::cout << "\nEjecutando búsqueda greedy desde nodo " << node2->id
              << " hasta nodo " << node6->id << std::endl;

    auto result = index.greedySearch(node2, node6, 3, 3);

    std::cout << "\nNodos más cercanos encontrados:" << std::endl;
    for (const auto &node: result.first) {
        std::cout << "Nodo " << node->id << " (distancia a nodo 6: "
                  << index.distance(node, node6) << ")" << std::endl;
    }

    std::cout << "\nTodos los nodos candidatos:" << std::endl;
    for (const auto &node: result.second) {
        std::cout << "Nodo " << node->id << " (distancia a nodo 6: "
                  << index.distance(node, node6) << ")" << std::endl;
    }
}

void testRobustPrune() {
    // Crear nodos con características específicas
    std::vector<double> values = {1.0, 2.0, 3.0};
    std::shared_ptr<GraphNode> node1 = std::make_shared<GraphNode>(1, values);
    values = {4.0, 5.0, 6.0};
    std::shared_ptr<GraphNode> node2 = std::make_shared<GraphNode>(2, values);
    values = {7.0, 8.0, 9.0};
    std::shared_ptr<GraphNode> node3 = std::make_shared<GraphNode>(3, values);
    values = {1.0, 3.0, 5.0};
    std::shared_ptr<GraphNode> node4 = std::make_shared<GraphNode>(4, values);
    values = {2.0, 4.0, 6.0};
    std::shared_ptr<GraphNode> node5 = std::make_shared<GraphNode>(5, values);
    values = {3.0, 5.0, 7.0};
    std::shared_ptr<GraphNode> node6 = std::make_shared<GraphNode>(6, values);

    // Conectar los nodos
    node1->outNeighbors.push_back(node2);
    node1->outNeighbors.push_back(node3);
    node1->outNeighbors.push_back(node4);
    node1->outNeighbors.push_back(node5);
    node1->outNeighbors.push_back(node6);

    std::vector<std::shared_ptr<GraphNode>> candidates = {node2, node3, node4, node5, node6};

    // Crear el índice
    FreshVamanaIndex index;

    // Ejecutar robustPrune
    double alpha = 1.5;
    size_t outDegreeBound = 3;
    index.robustPrune(node1, candidates, alpha, outDegreeBound);

    // Mostrar los vecinos de node1 después de la poda
    std::cout << "Vecinos de Node 1 después de robustPrune:" << std::endl;
    for (auto neighbor: node1->outNeighbors) {
        std::cout << "Node " << neighbor->id << std::endl;
    }
}

void testInsert() {
    std::vector<double> values = {1.0, 2.0, 3.0};
    std::shared_ptr<GraphNode> node1 = std::make_shared<GraphNode>(1, values);
    values = {4.0, 5.0, 6.0};
    std::shared_ptr<GraphNode> node2 = std::make_shared<GraphNode>(2, values);
    values = {7.0, 8.0, 9.0};
    std::shared_ptr<GraphNode> node3 = std::make_shared<GraphNode>(3, values);
    values = {1.0, 3.0, 5.0};
    std::shared_ptr<GraphNode> node4 = std::make_shared<GraphNode>(4, values);
    values = {2.0, 4.0, 6.0};
    std::shared_ptr<GraphNode> node5 = std::make_shared<GraphNode>(5, values);
    values = {3.0, 5.0, 7.0};
    std::shared_ptr<GraphNode> node6 = std::make_shared<GraphNode>(6, values);
    std::vector<std::shared_ptr<GraphNode>> nodes = {node1, node2, node3, node4, node5, node6};
    FreshVamanaIndex index;
    for (auto node: nodes) {
        index.insert(node);
    }

    // test 1-NN para cada nodo
    for (auto node: nodes) {
        auto result = index.knnSearch(node, 2);
        std::cout << "Nodos más cercanos al nodo " << node->id << ":" << " ";
        for (auto closest: result) {
            std::cout << "Nodo " << closest->id << " (distancia a nodo " << node->id << ": "
                      << index.distance(closest, node) << ")" << std::endl;
        }
    }

}


int main() {
#ifdef DEBUG
    std::cout << "Testing Node Creation..." << std::endl;
    testNodeCreation();

    std::cout << "\nTesting Distance Function..." << std::endl;
    testDistanceFunction();

    std::cout << "\nTesting Greedy Search..." << std::endl;
    testGreedySearch();

    std::cout << "\nTesting Robust Prune..." << std::endl;
    testRobustPrune();

    std::cout << "\nTesting Insert and Search" << std::endl;
    testInsert();
#else
    // Test parameters
    const size_t NEIGHBOR_COUNT = 5,
            SEARCH_LIST_SIZE = 75,
            N_TEST_POINTS = 5,
            OUT_DEGREE_BOUND = 64;
    const double ALPHA = 1.2;

    FreshVamanaIndex index(ALPHA, OUT_DEGREE_BOUND);

    auto dataset = FreshVamanaTestUtils::loadDataset("siftsmall_base.csv");

    std::cout << "Loaded dataset with " << dataset.size() << " entries" << std::endl;

    auto time = FreshVamanaTestUtils::time_function([&]() {
        for (auto dataPoint: dataset) {
            index.insert(dataPoint);
        }
    });

    std::cout << "Insertion took " << time.duration << " ms" << std::endl;

    auto nearestMap = FreshVamanaTestUtils::loadNearestGroundTruth("siftsmall_groundtruth.csv");

    // Test queries
    auto queries = FreshVamanaTestUtils::loadDataset("siftsmall_query.csv");

    size_t testCnt = 0;
    for (auto queryPoint: queries) {
        auto timedResult = FreshVamanaTestUtils::time_function<std::vector<std::shared_ptr<GraphNode>>>([&]() {
            return index.knnSearch(queryPoint, NEIGHBOR_COUNT, SEARCH_LIST_SIZE);
        });
        std::cout << "Search took " << timedResult.duration << " ms" << std::endl;
        // Verify search correctness
        auto trueNeighbors = nearestMap[queryPoint->id];
        std::set<size_t> trueNeighborSet(trueNeighbors.begin(), trueNeighbors.end());
        size_t positiveCount = 0;
        std::cout << "Neighbors for " << queryPoint->id << ": " << std::endl;
        for (size_t i = 0; i < NEIGHBOR_COUNT; ++i) {
            size_t foundNeighbor = timedResult.result[i]->id - 1;
            std::cout << i + 1 << ": " << "(TRUE) " << trueNeighbors[i] << " with distance " << "(FOUND) "
                      << foundNeighbor << std::endl;
            // Count for recall
            if (trueNeighborSet.find(foundNeighbor) != trueNeighborSet.end()) {
                positiveCount++;
            }
        }
        std::cout << "recall@" << NEIGHBOR_COUNT << ": " << (positiveCount)
                  << std::endl;

        testCnt++;
        if (testCnt == N_TEST_POINTS) {
            break;
        }
    }
#endif

    return 0;
}