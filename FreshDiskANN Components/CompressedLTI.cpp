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
    const std::vector<std::vector<double>>& data, size_t k) {

    size_t n = data.size();        // Número de puntos
    size_t d = data[0].size();     // Dimensionalidad de los puntos
    std::vector<std::vector<double>> centroids;
    std::vector<double> minDistances(n, std::numeric_limits<double>::max());

    // Inicializar el generador aleatorio
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, n - 1);

    // Seleccionar el primer centroide al azar
    centroids.push_back(data[distrib(gen)]);

    for (size_t i = 1; i < k; ++i) {
        // Calcular las distancias mínimas al centroide más cercano
        for (size_t j = 0; j < n; ++j) {
            double dist = 0.0;
            for (size_t dim = 0; dim < d; ++dim) {
                double diff = data[j][dim] - centroids.back()[dim];
                dist += diff * diff;
            }
            minDistances[j] = std::min(minDistances[j], dist);
        }

        // Seleccionar el siguiente centroide de forma proporcional a las distancias
        double totalDistance = 0.0;
        for (double dist : minDistances) {
            totalDistance += dist;
        }

        std::uniform_real_distribution<double> distDistrib(0, totalDistance);
        double target = distDistrib(gen);
        double cumulativeDistance = 0.0;

        size_t nextCentroidIndex = 0;
        for (size_t j = 0; j < n; ++j) {
            cumulativeDistance += minDistances[j];
            if (cumulativeDistance >= target) {
                nextCentroidIndex = j;
                break;
            }
        }

        centroids.push_back(data[nextCentroidIndex]);
    }

    return centroids;
}

// Función k-means con inicialización k-means++
std::vector<std::vector<double>> kmeans(
    const std::vector<std::vector<double>>& data, size_t k, size_t maxIterations = 100) {

    if (data.empty() || data[0].empty()) {
        throw std::invalid_argument("Data must not be empty and should contain at least one dimension.");
    }

    size_t n = data.size();        // Número de puntos
    size_t d = data[0].size();     // Dimensionalidad de los puntos

    // Generador aleatorio para centroides
    std::random_device rd;
    std::mt19937 gen(rd()); // Generador de números aleatorios
    std::uniform_int_distribution<size_t> distrib(0, n - 1);

    // Inicializar centroides con k-means++
    std::vector<std::vector<double>> centroids = initializeCentroidsPlusPlus(data, k);

    std::vector<size_t> assignments(n, 0);
    bool changed = true;
    size_t iterations = 0;

    while (changed && iterations < maxIterations) {
        changed = false;
        ++iterations;

        // Paso 1: Asignar cada punto al centroide más cercano
        for (size_t i = 0; i < n; ++i) {
            double minDist = std::numeric_limits<double>::max();
            size_t bestCentroid = 0;

            for (size_t j = 0; j < k; ++j) {
                double dist = 0.0;
                for (size_t dim = 0; dim < d; ++dim) {
                    double diff = data[i][dim] - centroids[j][dim];
                    dist += diff * diff;
                }

                if (dist < minDist) {
                    minDist = dist;
                    bestCentroid = j;
                }
            }

            if (assignments[i] != bestCentroid) {
                changed = true;
                assignments[i] = bestCentroid;
            }
        }

        // Paso 2: Actualizar los centroides
        std::vector<std::vector<double>> newCentroids(k, std::vector<double>(d, 0.0));
        std::vector<size_t> counts(k, 0);

        for (size_t i = 0; i < n; ++i) {
            size_t cluster = assignments[i];
            for (size_t dim = 0; dim < d; ++dim) {
                newCentroids[cluster][dim] += data[i][dim];
            }
            ++counts[cluster];
        }

        for (size_t j = 0; j < k; ++j) {
            if (counts[j] > 0) {
                for (size_t dim = 0; dim < d; ++dim) {
                    newCentroids[j][dim] /= counts[j];
                }
            } else {
                // Si un centroide queda vacío, reasígnalo usando k-means++
                newCentroids[j] = data[distrib(gen)];
            }
        }

        centroids = newCentroids;
    }

    return centroids;
}

std::vector<std::vector<size_t>> CompressedLTI::trainCentroidsPQ(
    const std::vector<std::pair<double, std::vector<std::vector<double>>>>& splitData,
    size_t k) {

    if (splitData.empty() || splitData[0].second.empty()) {
        throw std::invalid_argument("Input data must not be empty and should contain subvectors.");
    }

    size_t numSubvectors = splitData[0].second.size();
    centroidsPerSubvector.clear();
    centroidsPerSubvector.resize(numSubvectors);

    std::vector<std::vector<size_t>> compressedData(
        splitData.size(), std::vector<size_t>(numSubvectors));

    for (size_t i = 0; i < numSubvectors; ++i) {
        std::vector<std::vector<double>> currentSubvectorData;

        // Extraer el i-ésimo subvector de cada dato
        for (const auto& vector : splitData) {
            currentSubvectorData.push_back(vector.second[i]);
        }

        // Realiza K-Means para este subvector
        auto centroids = kmeans(currentSubvectorData, k);

        // Almacenar los centroides calculados para este subvector
        centroidsPerSubvector[i] = centroids;

        // Asignar el índice del centroide más cercano para cada subvector
        for (size_t j = 0; j < currentSubvectorData.size(); ++j) {
            double minDist = std::numeric_limits<double>::max();
            size_t closestCentroid = 0;

            for (size_t c = 0; c < centroids.size(); ++c) {
                double dist = 0.0;
                for (size_t d = 0; d < currentSubvectorData[j].size(); ++d) {
                    double diff = currentSubvectorData[j][d] - centroids[c][d];
                    dist += diff * diff;
                }

                if (dist < minDist) {
                    minDist = dist;
                    closestCentroid = c;
                }
            }

            compressedData[j][i] = closestCentroid;
        }
    }
    return compressedData;
}



double calculateDistance(const std::vector<double>& point, double scalar) {
    double distance = 0.0;
    for (size_t i = 0; i < point.size(); i++) {
        distance += (point[i] - scalar) * (point[i] - scalar);
    }
    return std::sqrt(distance);
};

std::vector<size_t> CompressedLTI::encodeVector(const std::vector<double>& vector) {
    size_t m = centroidsPerSubvector.size(); // Número de subvectores
    size_t subvectorDim = vector.size() / m;
    std::vector<size_t> encodedVector(m);

    for (size_t i = 0; i < m; ++i) {
        double minDist = std::numeric_limits<double>::max();
        size_t bestIndex = 0;

        for (size_t j = 0; j < centroidsPerSubvector[i].size(); ++j) {
            double dist = 0.0;
            for (size_t dim = 0; dim < subvectorDim; ++dim) {
                double diff = vector[i * subvectorDim + dim] - centroidsPerSubvector[i][j][dim];
                dist += diff * diff;
            }
            if (dist < minDist) {
                minDist = dist;
                bestIndex = j;
            }
        }
        encodedVector[i] = bestIndex;
    }

    return encodedVector;
}
double calculateApproxDistance(const std::vector<size_t>& queryIndices,
                               const std::vector<size_t>& dataIndices,
                               const std::vector<std::vector<std::vector<double>>>& centroids) {
    double distance = 0.0;
    for (size_t i = 0; i < queryIndices.size(); ++i) {
        auto& queryCentroid = centroids[i][queryIndices[i]];
        auto& dataCentroid = centroids[i][dataIndices[i]];
        for (size_t d = 0; d < queryCentroid.size(); ++d) {
            double diff = queryCentroid[d] - dataCentroid[d];
            distance += diff * diff;
        }
    }
    return std::sqrt(distance);
}


void CompressedLTI::storeCompressedGraphNodes(const std::vector<std::pair<int, std::vector<size_t>>>& encodedData) {
    for (const auto& [id, encodedVector] : encodedData) {
        auto node = std::make_shared<CompressedGraphNode>();
        node->id = id;

        // Comprimir los índices en un vector de bytes
        node->compressedFeatures.resize(encodedVector.size());
        std::transform(encodedVector.begin(), encodedVector.end(), node->compressedFeatures.begin(),
                       [](size_t index) { return static_cast<uint8_t>(index); });

        compressedGraphNodes[id] = node;
    }
}



std::vector<int> CompressedLTI::knnSearch(
    const std::vector<unsigned long>& query,
    const std::vector<std::vector<unsigned long>>& encodedDataset,
    int kNearestNeighbors){
    std::vector<std::pair<int, double>> distances;

    // Calcular la distancia entre el query y cada elemento del dataset
    for (int i = 0; i < encodedDataset.size(); i++) {
        double distance = 0.0;
        for (int j = 0; j < query.size(); j++) {
            distance += std::pow(query[j] - encodedDataset[i][j], 2);
        }
        distance = std::sqrt(distance);
        distances.push_back(std::make_pair(i, distance));
    }

    // Ordenar las distancias de menor a mayor
    std::sort(distances.begin(), distances.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second < b.second;
    });

    // Obtener los k vecinos más cercanos
    std::vector<int> kNearest;
    for (int i = 0; i < kNearestNeighbors; i++) {
        kNearest.push_back(distances[i].first);
    }

    return kNearest;
}

std::vector<size_t> encodeVector(
    const std::vector<double>& vector,
    const std::vector<std::vector<std::vector<double>>>& centroids) {

    size_t m = centroids.size(); // Número de subvectores
    size_t subvectorDim = vector.size() / m;
    std::vector<size_t> encodedVector(m);

    for (size_t i = 0; i < m; ++i) {
        double minDist = std::numeric_limits<double>::max();
        size_t bestIndex = 0;

        for (size_t j = 0; j < centroids[i].size(); ++j) {
            double dist = 0.0;
            for (size_t dim = 0; dim < subvectorDim; ++dim) {
                double diff = vector[i * subvectorDim + dim] - centroids[i][j][dim];
                dist += diff * diff;
            }
            if (dist < minDist) {
                minDist = dist;
                bestIndex = j;
            }
        }

        encodedVector[i] = bestIndex;
    }

    return encodedVector;
}