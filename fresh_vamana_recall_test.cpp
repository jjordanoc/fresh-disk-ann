// main.cpp
#include <iostream>
#include <chrono>
#include <set>
#include <map>
#include <cassert>
#include "FreshVamanaTestUtils.hpp"
#include "FreshVamanaIndex.h"




int main() {
    // Test parameters (annotated paper values)
    const size_t NEIGHBOR_COUNT = 5, // 5
    SEARCH_LIST_SIZE = 30, // 75
    N_TEST_POINTS = 50,
            OUT_DEGREE_BOUND = 37; // 64
    const double ALPHA = 1.2, // 1.2
    DELETE_ACCUMULATION_FACTOR = 0.04, // 0.1
    PERCENTAGE_REMOVED = 0.5; // 0.05

    FreshVamanaIndex index(ALPHA, OUT_DEGREE_BOUND, DELETE_ACCUMULATION_FACTOR);

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
    // use only a subset of the queries for this test
    auto rng = std::default_random_engine{};
    std::shuffle(queries.begin(), queries.end(), rng);
    queries.resize(N_TEST_POINTS);

    // results file
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << "results-" << OUT_DEGREE_BOUND << "-" << PERCENTAGE_REMOVED << "-" << ALPHA << ".csv";
    std::string s = stream.str();
    std::string filename = stream.str();
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file " << "results.csv" << std::endl;
        return -1;
    }

    std::cout << std::setprecision(5);
    outfile << std::setprecision(5);

    double removeThreshold = PERCENTAGE_REMOVED * (double) dataset.size();

    // Test cycles of insertion / deletion
    size_t cycle = 0;
    std::map<size_t, std::shared_ptr<GraphNode>> removed;
    while (cycle < 20) {
        // Insert and delete random points
        auto randomPoint = FreshVamanaTestUtils::pickRandomPoint(dataset, removed);
        index.deleteNode(randomPoint);
        removed.insert({randomPoint->id, randomPoint});

        // assert removal
//        for (const auto &node : index.graph) {
//            if (node->deleted) {
//                if (index.deleteList.find(node) != index.deleteList.end()) {
//                    continue;
//                }
//                if (removed.find(node->id) == removed.end()) {
//                    // node should be in the graph
//                    std::cout << node->id << std::endl;
//                    std::cout << index.graph.size() << std::endl;
//                }
//            }
//        }
        if (removed.size() % (int) (removeThreshold / 5) == 0) {
            // compute recall
            double avgRecall = 0;
            for (auto queryPoint: queries) {
                auto timedResult = FreshVamanaTestUtils::time_function<std::vector<std::shared_ptr<GraphNode>>>([&]() {
                    return index.knnSearch(queryPoint, NEIGHBOR_COUNT, SEARCH_LIST_SIZE);
                });
//            std::cout << "Search took " << timedResult.duration << " ms" << std::endl;
                // Verify search correctness
                auto trueNeighbors = nearestMap[queryPoint->id];
                std::set<size_t> trueNeighborSet;
                for (const auto &actualNeighbor: trueNeighbors) {
                    if (removed.find(actualNeighbor) == removed.end()) {
                        trueNeighborSet.insert(actualNeighbor);
                        if (trueNeighborSet.size() == NEIGHBOR_COUNT) {
                            break;
                        }
                    }
                }
                std::vector<size_t> trueNeighborVec{trueNeighborSet.begin(), trueNeighborSet.end()};
                size_t positiveCount = 0;
//            std::cout << "Neighbors for " << queryPoint->id << ": " << std::endl;
                for (size_t i = 0; i < NEIGHBOR_COUNT; ++i) {
                    size_t foundNeighbor = timedResult.result[i]->id;
                    auto foundNeighborNode = timedResult.result[i];

                    auto trueNeighborNode = index.getNode(trueNeighborVec[i]);
                    // assert
                    assert(trueNeighborNode != nullptr);
                    if (trueNeighborNode == nullptr) {
                        std::cout << trueNeighborVec[i] << std::endl;
                        std::cout << (removed.find(trueNeighborVec[i]) == removed.end()) << std::endl;
                        std::cout << index.graph.size() << std::endl;
                        std::cout << index.deleteList.size() << std::endl;
                    }

//                std::cout << i + 1 << ": " << "(TRUE) " << trueNeighborVec[i] << " with distance "
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
            }
            double progress = (double) cycle + ((double) removed.size() / removeThreshold);
            avgRecall = (avgRecall / (double) N_TEST_POINTS);
            std::cout << "progress: " << progress << " avg recall@" << NEIGHBOR_COUNT << ": " << avgRecall
                      << std::endl;

            // write to file
            outfile << progress << "," << avgRecall << "\n";
        }


        if (removed.size() >= removeThreshold) {
            std::cout << "Finished cycle " << cycle << " . Reinserting..." << std::endl;
            while (!removed.empty()) {
                auto removedNode  = removed.begin()->second;
                // make sure node is not marked as deleted
                removedNode->deleted = false;
                if (index.deleteList.find(removedNode) == index.deleteList.end()) {
                    index.insert(removedNode);
                }
//                else {
//                    // assert
//                    // dont reinsert deleted nodes (shouldnt happen with 0.04 delete factor)
//                    std::cout << "assert delete factor" << std::endl;
//                    std::cout << removedNode->id << std::endl;
//                    std::cout << index.graph.size() << std::endl;
//                    std::cout << index.deleteList.size() << std::endl;
//                }
                removed.erase(removed.begin());
            }
            assert(index.graph.size() == dataset.size());
            cycle++;
        }
    }
    outfile.close();


    return 0;
}