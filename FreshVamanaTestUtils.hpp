#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "GraphNode.h"

namespace FreshVamanaTestUtils {
    std::vector<GraphNode> loadDataset(std::string path) {
        std::vector<GraphNode> data;
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
                } catch (const std::invalid_argument&) {
                    throw std::runtime_error("Invalid number in file: " + value);
                }
            }

            if (!row.empty()) {
                data.push_back(GraphNode(id, row));
                id++;
            }
        }

        return data;
    }
}