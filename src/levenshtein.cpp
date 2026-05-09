#include "levenshtein.h"

#include "CleanUp.h"

#include <algorithm>

namespace {
std::vector<std::string> getCleanDevUnits(const std::string& text) {
    std::vector<std::string> units;
    for (const auto& unit : splitUTF8(text)) {
        if (unit.empty() || isIgnorableSymbol(unit)) {
            continue;
        }
        units.push_back(unit);
    }
    return units;
}

std::vector<std::string> getCleanIASTUnits(const std::string& text) {
    std::vector<std::string> units;
    for (const auto& unit : splitIAST(text)) {
        if (unit.empty() || isIgnorableSymbol(unit)) {
            continue;
        }
        units.push_back(unit);
    }
    return units;
}

std::string joinHymnDevText(const Hymn& hymn) {
    std::string text;
    for (const auto& verse : hymn.getVerses()) {
        if (!text.empty()) {
            text += " ";
        }
        text += verse.getDev();
    }
    return text;
}

std::string joinHymnIASTText(const Hymn& hymn) {
    std::string text;
    for (const auto& verse : hymn.getVerses()) {
        if (!text.empty()) {
            text += " ";
        }
        text += verse.getIAST();
    }
    return text;
}
}

int computeLevenshteinDistance(
    const std::vector<std::string>& left,
    const std::vector<std::string>& right) {
    if (left.empty()) {
        return static_cast<int>(right.size());
    }
    if (right.empty()) {
        return static_cast<int>(left.size());
    }

    std::vector<int> previous(right.size() + 1);
    std::vector<int> current(right.size() + 1);

    for (size_t j = 0; j <= right.size(); ++j) {
        previous[j] = static_cast<int>(j);
    }

    for (size_t i = 1; i <= left.size(); ++i) {
        current[0] = static_cast<int>(i);
        for (size_t j = 1; j <= right.size(); ++j) {
            const int substitutionCost = left[i - 1] == right[j - 1] ? 0 : 1;
            current[j] = std::min({
                previous[j] + 1,
                current[j - 1] + 1,
                previous[j - 1] + substitutionCost
            });
        }
        std::swap(previous, current);
    }

    return previous.back();
}

LevenshteinMetrics buildLevenshteinMetrics(
    const std::vector<std::string>& left,
    const std::vector<std::string>& right) {
    LevenshteinMetrics metrics;
    metrics.distance = computeLevenshteinDistance(left, right);

    const size_t baseline = std::max(left.size(), right.size());
    if (baseline == 0) {
        metrics.normalizedDistance = 0.0;
        metrics.similarity = 1.0;
        return metrics;
    }

    metrics.normalizedDistance =
        static_cast<double>(metrics.distance) / static_cast<double>(baseline);
    metrics.similarity = 1.0 - metrics.normalizedDistance;
    return metrics;
}

LevenshteinMetrics computeDevLevenshteinMetrics(const Verse& left, const Verse& right) {
    return buildLevenshteinMetrics(getCleanDevUnits(left.getDev()), getCleanDevUnits(right.getDev()));
}

LevenshteinMetrics computeIASTLevenshteinMetrics(const Verse& left, const Verse& right) {
    return buildLevenshteinMetrics(getCleanIASTUnits(left.getIAST()), getCleanIASTUnits(right.getIAST()));
}

LevenshteinMetrics computeDevLevenshteinMetrics(const Hymn& left, const Hymn& right) {
    return buildLevenshteinMetrics(getCleanDevUnits(joinHymnDevText(left)), getCleanDevUnits(joinHymnDevText(right)));
}

LevenshteinMetrics computeIASTLevenshteinMetrics(const Hymn& left, const Hymn& right) {
    return buildLevenshteinMetrics(getCleanIASTUnits(joinHymnIASTText(left)), getCleanIASTUnits(joinHymnIASTText(right)));
}
