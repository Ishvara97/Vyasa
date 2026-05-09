#ifndef MATRIX_ANALYSIS_H
#define MATRIX_ANALYSIS_H

#include "../PoemStructures.h"
#include <map>
#include <string>
#include <vector>

enum class MatrixTokenMode {
    Character,
    PhonemeClass
};

struct SyllableFeatureRow {
    int verseNumber = 0;
    int wordIndex = 0;
    int syllableIndex = 0;
    std::string syllable;
    std::string onsetClass;
    std::string nucleusBase;
    std::string codaClass;
    std::string weight;
    std::string swara;
};

using CountMatrix = std::map<std::string, std::map<std::string, int>>;
using ProbabilityMatrix = std::map<std::string, std::map<std::string, double>>;

CountMatrix buildCooccurrenceMatrix(const Verse& verse, MatrixTokenMode mode, int windowRadius);
CountMatrix buildCooccurrenceMatrix(const Hymn& hymn, MatrixTokenMode mode, int windowRadius);
CountMatrix buildPhonemeClassTransitionMatrix(const Verse& verse);
CountMatrix buildPhonemeClassTransitionMatrix(const Hymn& hymn);
ProbabilityMatrix buildRowNormalizedProbabilityMatrix(const CountMatrix& counts);
std::vector<SyllableFeatureRow> buildSyllableFeatureMatrix(const Verse& verse);
std::vector<SyllableFeatureRow> buildSyllableFeatureMatrix(const Hymn& hymn);

void exportCountMatrixCSV(
    const CountMatrix& matrix,
    const std::string& filename,
    const std::string& rowLabel,
    const std::string& columnLabel,
    bool includeWideMatrix = true);
void exportTransitionMatrixCSV(
    const CountMatrix& counts,
    const std::string& filename);
void exportSyllableFeatureMatrixCSV(
    const std::vector<SyllableFeatureRow>& rows,
    const std::string& filename);

#endif
