#include "similarity.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace {
template <typename T>
std::pair<std::vector<double>, std::vector<double>> alignFeatureVectorsImpl(
    const std::map<std::string, T>& left,
    const std::map<std::string, T>& right) {
    std::set<std::string> keys;
    for (const auto& [key, value] : left) {
        (void)value;
        keys.insert(key);
    }
    for (const auto& [key, value] : right) {
        (void)value;
        keys.insert(key);
    }

    std::vector<double> leftVector;
    std::vector<double> rightVector;
    leftVector.reserve(keys.size());
    rightVector.reserve(keys.size());

    for (const auto& key : keys) {
        const auto leftIt = left.find(key);
        const auto rightIt = right.find(key);
        leftVector.push_back(leftIt != left.end() ? static_cast<double>(leftIt->second) : 0.0);
        rightVector.push_back(rightIt != right.end() ? static_cast<double>(rightIt->second) : 0.0);
    }

    return {leftVector, rightVector};
}
}

double computeDotProduct(const std::vector<double>& left, const std::vector<double>& right) {
    const size_t count = std::min(left.size(), right.size());
    double total = 0.0;

    for (size_t i = 0; i < count; ++i) {
        total += left[i] * right[i];
    }

    return total;
}

double computeVectorMagnitude(const std::vector<double>& values) {
    double squaredTotal = 0.0;

    for (double value : values) {
        squaredTotal += value * value;
    }

    return std::sqrt(squaredTotal);
}

double computeCosineSimilarity(const std::vector<double>& left, const std::vector<double>& right) {
    const double magnitudeLeft = computeVectorMagnitude(left);
    const double magnitudeRight = computeVectorMagnitude(right);

    if (magnitudeLeft == 0.0 || magnitudeRight == 0.0) {
        return 0.0;
    }

    return computeDotProduct(left, right) / (magnitudeLeft * magnitudeRight);
}

double computeSimilarityConfidence(const std::vector<double>& left, const std::vector<double>& right) {
    const size_t count = std::min(left.size(), right.size());
    if (count == 0) {
        return 0.0;
    }

    double leftTotal = 0.0;
    double rightTotal = 0.0;
    size_t activeDimensions = 0;
    size_t sharedDimensions = 0;

    for (size_t i = 0; i < count; ++i) {
        const double leftValue = std::abs(left[i]);
        const double rightValue = std::abs(right[i]);

        leftTotal += leftValue;
        rightTotal += rightValue;

        if (leftValue > 0.0 || rightValue > 0.0) {
            ++activeDimensions;
        }
        if (leftValue > 0.0 && rightValue > 0.0) {
            ++sharedDimensions;
        }
    }

    if (leftTotal == 0.0 || rightTotal == 0.0 || activeDimensions == 0) {
        return 0.0;
    }

    const double overlapRatio =
        static_cast<double>(sharedDimensions) / static_cast<double>(activeDimensions);
    const double balanceRatio = std::min(leftTotal, rightTotal) / std::max(leftTotal, rightTotal);
    const double evidenceRatio = std::min(1.0, static_cast<double>(activeDimensions) / 8.0);

    return (overlapRatio + balanceRatio + evidenceRatio) / 3.0;
}

SimilarityScore buildSimilarityScore(const std::vector<double>& left, const std::vector<double>& right) {
    SimilarityScore score;
    score.dotProduct = computeDotProduct(left, right);
    score.magnitudeA = computeVectorMagnitude(left);
    score.magnitudeB = computeVectorMagnitude(right);
    score.cosineSimilarity = computeCosineSimilarity(left, right);
    score.confidence = computeSimilarityConfidence(left, right);
    return score;
}

std::pair<std::vector<double>, std::vector<double>> alignFeatureVectors(
    const std::map<std::string, int>& left,
    const std::map<std::string, int>& right) {
    return alignFeatureVectorsImpl(left, right);
}

std::pair<std::vector<double>, std::vector<double>> alignFeatureVectors(
    const std::map<std::string, double>& left,
    const std::map<std::string, double>& right) {
    return alignFeatureVectorsImpl(left, right);
}
