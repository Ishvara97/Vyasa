#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#include "../PoemStructures.h"
#include <string>
#include <vector>

struct LevenshteinMetrics {
    int distance = 0;
    double normalizedDistance = 0.0;
    double similarity = 1.0;
};

int computeLevenshteinDistance(
    const std::vector<std::string>& left,
    const std::vector<std::string>& right);
LevenshteinMetrics buildLevenshteinMetrics(
    const std::vector<std::string>& left,
    const std::vector<std::string>& right);
LevenshteinMetrics computeDevLevenshteinMetrics(const Verse& left, const Verse& right);
LevenshteinMetrics computeIASTLevenshteinMetrics(const Verse& left, const Verse& right);
LevenshteinMetrics computeDevLevenshteinMetrics(const Hymn& left, const Hymn& right);
LevenshteinMetrics computeIASTLevenshteinMetrics(const Hymn& left, const Hymn& right);

#endif
