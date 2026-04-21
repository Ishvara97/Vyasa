#ifndef SIMILARITY_H
#define SIMILARITY_H

#include <map>
#include <string>
#include <utility>
#include <vector>

struct SimilarityScore {
    double dotProduct = 0.0;
    double magnitudeA = 0.0;
    double magnitudeB = 0.0;
    double cosineSimilarity = 0.0;
    double confidence = 0.0;
};

double computeDotProduct(const std::vector<double>& left, const std::vector<double>& right);
double computeVectorMagnitude(const std::vector<double>& values);
double computeCosineSimilarity(const std::vector<double>& left, const std::vector<double>& right);
double computeSimilarityConfidence(const std::vector<double>& left, const std::vector<double>& right);
SimilarityScore buildSimilarityScore(const std::vector<double>& left, const std::vector<double>& right);

std::pair<std::vector<double>, std::vector<double>> alignFeatureVectors(
    const std::map<std::string, int>& left,
    const std::map<std::string, int>& right);
std::pair<std::vector<double>, std::vector<double>> alignFeatureVectors(
    const std::map<std::string, double>& left,
    const std::map<std::string, double>& right);

#endif
