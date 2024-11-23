#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <unordered_map>
#include "GraphNode.h"

namespace FreshVamanaTestUtils {
    std::vector<std::shared_ptr<GraphNode>> loadDataset(std::string path) {
        std::vector<std::shared_ptr<GraphNode>> data;
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + path);
        }
        size_t id = 1;
        std::string line;
        while (std::getline(file, line)) {
            std::vector<double> row;
            std::string value;
            std::istringstream stream(line);

            while (std::getline(stream, value, ',')) { // Use ',' as the delimiter
                try {
                    row.push_back(std::stod(value)); // Convert string to double
                } catch (const std::invalid_argument &) {
                    throw std::runtime_error("Invalid number in file: " + value);
                }
            }
            if (!row.empty()) {
                data.push_back(std::make_shared<GraphNode>(id, row));
                id++;
            }
        }
        return data;
    }

    std::unordered_map<size_t, std::vector<size_t>> loadNearestGroundTruth(std::string path) {
        std::unordered_map<size_t, std::vector<size_t>> data;
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + path);
        }
        size_t id = 1;
        std::string line;
        while (std::getline(file, line)) {
            std::vector<size_t> row;
            std::string value;
            std::istringstream stream(line);

            while (std::getline(stream, value, ',')) { // Use ',' as the delimiter
                try {
                    row.push_back(std::stoi(value)); // Convert string to double
                } catch (const std::invalid_argument &) {
                    throw std::runtime_error("Invalid number in file: " + value);
                }
            }
            if (!row.empty()) {
                data[id] = row;
                id++;
            }
        }
        return data;
    }

    template<typename Return>
    struct TimedResult
    {
        Return result;
        size_t duration;
        TimedResult(Return &_result, size_t &_duration) : result(_result), duration(_duration) {}
    };

    template<>
    struct TimedResult<void> {
        size_t duration;
        TimedResult(size_t &_duration) : duration(_duration) {}
    };

    template<typename Return, typename Fun, typename ...Args>
    TimedResult<Return> time_function(const Fun &function, Args... args) {
        auto start = std::chrono::steady_clock ::now();
        Return result = function(args...);
        auto end = std::chrono::steady_clock::now();
        size_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return {result, duration};
    }

    template<typename Fun, typename ...Args>
    TimedResult<void> time_function(const Fun &function, Args... args) {
        auto start = std::chrono::steady_clock::now();
        function(args...);
        auto end = std::chrono::steady_clock::now();
        size_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return {duration};
    }

    std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> computeExcludedNodeList(std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp>, size_t k) {
        std::set<std::shared_ptr<GraphNode>, GraphNode::SharedPtrComp> excluded;
        for (auto &[id, neighbors] : neighborMap) {
            neighbors.resize(k);
        }

    }

    std::shared_ptr<GraphNode> pickRandomPoint(const std::vector<std::shared_ptr<GraphNode>> &dataset) {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> uniform(0, dataset.size() - 1);
        return dataset[uniform(rng)];
    }

}