// main.cpp
#include <iostream>
#include <chrono>
#include <set>
#include "FreshVamanaTestUtils.hpp"
#include "FreshVamanaIndex.h"


int main() {
    // Test parameters (annotated paper values)
    const size_t NEIGHBOR_COUNT = 5, // 5
    SEARCH_LIST_SIZE = 30, // 75
    N_TEST_POINTS = 100,
            OUT_DEGREE_BOUND = 64; // 64
    const double ALPHA = 1.2; // 1.2

    FreshVamanaIndex index(ALPHA, OUT_DEGREE_BOUND);

    auto dataset = FreshVamanaTestUtils::loadDataset("siftsmall_base.csv");

    std::cout << "Loaded dataset with " << dataset.size() << " entries" << std::endl;

    auto time = FreshVamanaTestUtils::time_function([&]() {
        for (auto dataPoint: dataset) {
            index.insert(dataPoint, SEARCH_LIST_SIZE, false);
        }
    });

    std::cout << "Insertion took " << time.duration << " ms" << std::endl;

    auto nearestMap = FreshVamanaTestUtils::loadNearestGroundTruth("siftsmall_groundtruth.csv");
    auto queries = FreshVamanaTestUtils::loadDataset("siftsmall_query.csv");

    // Test cycles of insertion / deletion
    size_t cycle = 0;
    while (cycle < 20) {
        std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> removed;
        // Insert and delete random points
        auto randomPoint = FreshVamanaTestUtils::pickRandomPoint(dataset, removed);
        index.deleteNode(randomPoint);

        size_t testCnt = 0;
        double avgRecall = 0;
        for (auto queryPoint: queries) {
            auto timedResult = FreshVamanaTestUtils::time_function<std::vector<std::shared_ptr<GraphNode>>>([&]() {
                return index.knnSearch(queryPoint, NEIGHBOR_COUNT, SEARCH_LIST_SIZE);
            });
//            std::cout << "Search took " << timedResult.duration << " ms" << std::endl;
            // Verify search correctness
            auto trueNeighbors = nearestMap[queryPoint->id];
            trueNeighbors.resize(NEIGHBOR_COUNT);
            std::set<size_t> trueNeighborSet(trueNeighbors.begin(), trueNeighbors.end());
            size_t positiveCount = 0;
//            std::cout << "Neighbors for " << queryPoint->id << ": " << std::endl;
            for (size_t i = 0; i < NEIGHBOR_COUNT; ++i) {
                size_t foundNeighbor = timedResult.result[i]->id - 1;
                auto foundNeighborNode = timedResult.result[i];
                auto trueNeighborNode = index.getNode(trueNeighbors[i] + 1);
//                std::cout << i + 1 << ": " << "(TRUE) " << trueNeighbors[i] << " with distance "
//                          << index.distance(trueNeighborNode, queryPoint) << " (FOUND) "
//                          << foundNeighbor << " with distance " << index.distance(foundNeighborNode, queryPoint)
//                          << std::endl;
                // Count for recall
                if (trueNeighborSet.find(foundNeighbor) != trueNeighborSet.end()) {
                    positiveCount++;
                }
            }
            double recall = ((double) positiveCount / (double) NEIGHBOR_COUNT);
            avgRecall += recall;
//            std::cout << "recall@" << NEIGHBOR_COUNT << ": " << recall
//                      << std::endl;
            testCnt++;
            if (testCnt == N_TEST_POINTS) {
                break;
            }
        }
        std::cout << "avg recall@" << NEIGHBOR_COUNT << ": " << (avgRecall / (double) N_TEST_POINTS)
                  << std::endl;

        if (removed.size() >= 0.05 * dataset.size()) {
            std::cout << "Finished cycle " << cycle << " reinserting..." << std::endl;
            while (!removed.empty()) {
                index.insert(*(removed.begin()));
                removed.erase(removed.begin());
            }
            cycle++;

        }
    }




    return 0;
}