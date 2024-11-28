//
// Created by Juan Pedro on 22/11/2024.
//

#include "CompressedLTI.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <queue>
#include <iostream>
#include <Eigen/Dense>
#include <unordered_set>


// Método para preparar datos de entrenamiento
std::vector< std::pair<double,std::vector<double>>> CompressedLTI::prepareTrainingData(const std::vector<std::shared_ptr<GraphNode>>& dataset) {
    std::vector< std::pair<double,std::vector<double>>> trainingData;
    Eigen::MatrixXd dataMatrix(dataset.size(), dataset[0]->features.size());

    // Construir una matriz de datos
    for (int i = 0; i < dataset.size(); i++) {
        if (!dataset[i]->features.empty()) {
            for (int j = 0; j < dataset[i]->features.size(); j++) {
                dataMatrix(i, j) = dataset[i]->features[j];
            }
        }
    }

    // Calcular la media y la desviación estándar de cada característica
    Eigen::VectorXd mean = dataMatrix.colwise().mean();
    Eigen::VectorXd stdDev = ((dataMatrix.rowwise() - mean.transpose()).array().square().colwise().sum() / (dataMatrix.rows() - 1)).sqrt();

    // Normalizar los datos dividiendo cada valor de la característica por su desviación estándar
    for (int i = 0; i < dataset.size(); i++) {
        if (!dataset[i]->features.empty()) {
            std::vector<double> normalizedFeatures;
            for (int j = 0; j < dataset[i]->features.size(); j++) {
                double normalizedValue = (dataset[i]->features[j] - mean[j]) / stdDev[j];
                normalizedFeatures.push_back(normalizedValue);
            }
            trainingData.push_back(std::make_pair(dataset[i]->id, normalizedFeatures));
        }
    }

    return trainingData;
}

std::vector< std::pair<double,std::vector<std::vector<double>>>> CompressedLTI::splitVectors(
    const std::vector< std::pair<double,std::vector<double>>>& data, size_t m) {

    std::vector< std::pair<double,std::vector<std::vector<double>>>> splitData;
    size_t subvectorSize = data[0].second.size() / m;

    for (const auto& vector : data) {
        std::vector<std::vector<double>> subvectors;
        for (size_t i = 0; i < m; ++i) {
            std::vector<double> subvector(vector.second.begin() + i * subvectorSize,
                                          vector.second.begin() + (i + 1) * subvectorSize);
            subvectors.push_back(subvector);
        }
        splitData.push_back(std::make_pair(vector.first, subvectors));
    }

    return splitData;
}

double calculateDistance(const std::vector<double>& point1, const std::vector<double>& point2) {
    double distance = 0.0;
    for (size_t i = 0; i < point1.size(); i++) {
        distance += (point1[i] - point2[i]) * (point1[i] - point2[i]);
    }
    return std::sqrt(distance);
}

// Función para inicializar los centroides usando k-means++
std::vector<std::vector<double>> initializeCentroidsPlusPlus(
    const std::vector<