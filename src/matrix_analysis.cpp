#include "matrix_analysis.h"

#include "CleanUp.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace {
std::string joinLetters(const std::vector<Letter>& letters) {
    std::string value;
    for (const auto& letter : letters) {
        value += letter.getValue();
    }
    return value;
}

std::string syllableToText(const Syllable& syllable) {
    return joinLetters(syllable.getOnset()) +
           syllable.getNucleus().getValue() +
           joinLetters(syllable.getCoda());
}

std::string joinStrings(const std::vector<std::string>& values, const std::string& delim) {
    std::ostringstream out;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            out << delim;
        }
        out << values[i];
    }
    return out.str();
}

std::string csvEscape(const std::string& value) {
    std::string escaped = value;
    size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != std::string::npos) {
        escaped.insert(pos, 1, '"');
        pos += 2;
    }
    return "\"" + escaped + "\"";
}

std::string classifyLetterToken(const Letter& letter) {
    const std::string label = phonemeClassToString(letter.getPhoneme());
    return label == "Other" ? std::string() : label;
}

std::vector<std::string> collectVerseTokens(const Verse& verse, MatrixTokenMode mode) {
    std::vector<std::string> tokens;
    for (const auto& word : verse.getDevWords()) {
        for (const auto& letter : word.getLetters()) {
            if (isIgnorableSymbol(letter.getValue())) {
                continue;
            }

            if (mode == MatrixTokenMode::Character) {
                tokens.push_back(letter.getValue());
                continue;
            }

            const std::string label = classifyLetterToken(letter);
            if (!label.empty()) {
                tokens.push_back(label);
            }
        }
    }
    return tokens;
}

std::vector<std::string> collectHymnTokens(const Hymn& hymn, MatrixTokenMode mode) {
    std::vector<std::string> tokens;
    for (const auto& verse : hymn.getVerses()) {
        const auto verseTokens = collectVerseTokens(verse, mode);
        tokens.insert(tokens.end(), verseTokens.begin(), verseTokens.end());
    }
    return tokens;
}

CountMatrix buildCooccurrenceMatrixFromTokens(const std::vector<std::string>& tokens, int windowRadius) {
    CountMatrix matrix;
    if (windowRadius <= 0) {
        return matrix;
    }

    for (size_t i = 0; i < tokens.size(); ++i) {
        const size_t left = i > static_cast<size_t>(windowRadius)
            ? i - static_cast<size_t>(windowRadius)
            : 0;
        const size_t right = std::min(tokens.size() - 1, i + static_cast<size_t>(windowRadius));
        for (size_t j = left; j <= right; ++j) {
            if (i == j) {
                continue;
            }
            matrix[tokens[i]][tokens[j]]++;
        }
    }

    return matrix;
}

CountMatrix buildTransitionMatrixFromTokens(const std::vector<std::string>& tokens) {
    CountMatrix matrix;
    if (tokens.size() < 2) {
        return matrix;
    }

    for (size_t i = 0; i + 1 < tokens.size(); ++i) {
        matrix[tokens[i]][tokens[i + 1]]++;
    }
    return matrix;
}

std::string summarizeLetterClasses(const std::vector<Letter>& letters) {
    std::vector<std::string> classes;
    for (const auto& letter : letters) {
        const std::string label = classifyLetterToken(letter);
        if (!label.empty()) {
            classes.push_back(label);
        }
    }

    if (classes.empty()) {
        return "None";
    }

    return joinStrings(classes, "|");
}

std::string summarizeSwaras(const Syllable& syllable) {
    if (syllable.getSwaras().empty()) {
        return "None";
    }
    return joinStrings(syllable.getSwaras(), "|");
}

std::set<std::string> collectMatrixHeaders(const CountMatrix& matrix) {
    std::set<std::string> headers;
    for (const auto& [row, columns] : matrix) {
        headers.insert(row);
        for (const auto& [column, count] : columns) {
            (void)count;
            headers.insert(column);
        }
    }
    return headers;
}
}

CountMatrix buildCooccurrenceMatrix(const Verse& verse, MatrixTokenMode mode, int windowRadius) {
    return buildCooccurrenceMatrixFromTokens(collectVerseTokens(verse, mode), windowRadius);
}

CountMatrix buildCooccurrenceMatrix(const Hymn& hymn, MatrixTokenMode mode, int windowRadius) {
    return buildCooccurrenceMatrixFromTokens(collectHymnTokens(hymn, mode), windowRadius);
}

CountMatrix buildPhonemeClassTransitionMatrix(const Verse& verse) {
    return buildTransitionMatrixFromTokens(collectVerseTokens(verse, MatrixTokenMode::PhonemeClass));
}

CountMatrix buildPhonemeClassTransitionMatrix(const Hymn& hymn) {
    return buildTransitionMatrixFromTokens(collectHymnTokens(hymn, MatrixTokenMode::PhonemeClass));
}

ProbabilityMatrix buildRowNormalizedProbabilityMatrix(const CountMatrix& counts) {
    ProbabilityMatrix probabilities;
    for (const auto& [row, columns] : counts) {
        int total = 0;
        for (const auto& [column, count] : columns) {
            (void)column;
            total += count;
        }

        if (total <= 0) {
            continue;
        }

        for (const auto& [column, count] : columns) {
            probabilities[row][column] =
                static_cast<double>(count) / static_cast<double>(total);
        }
    }
    return probabilities;
}

std::vector<SyllableFeatureRow> buildSyllableFeatureMatrix(const Verse& verse) {
    std::vector<SyllableFeatureRow> rows;
    for (size_t wordIndex = 0; wordIndex < verse.getDevWords().size(); ++wordIndex) {
        const auto& word = verse.getDevWords()[wordIndex];
        const auto& syllables = word.getSyllables();
        for (size_t syllableIndex = 0; syllableIndex < syllables.size(); ++syllableIndex) {
            const auto& syllable = syllables[syllableIndex];
            rows.push_back({
                verse.getVerseNumber(),
                static_cast<int>(wordIndex + 1),
                static_cast<int>(syllableIndex + 1),
                syllableToText(syllable),
                summarizeLetterClasses(syllable.getOnset()),
                syllable.getNucleus().getPhoneme().base.empty() ? "None" : syllable.getNucleus().getPhoneme().base,
                summarizeLetterClasses(syllable.getCoda()),
                syllable.getWeight(),
                summarizeSwaras(syllable)
            });
        }
    }
    return rows;
}

std::vector<SyllableFeatureRow> buildSyllableFeatureMatrix(const Hymn& hymn) {
    std::vector<SyllableFeatureRow> rows;
    for (const auto& verse : hymn.getVerses()) {
        const auto verseRows = buildSyllableFeatureMatrix(verse);
        rows.insert(rows.end(), verseRows.begin(), verseRows.end());
    }
    return rows;
}

void exportCountMatrixCSV(
    const CountMatrix& matrix,
    const std::string& filename,
    const std::string& rowLabel,
    const std::string& columnLabel,
    bool includeWideMatrix) {
    std::ofstream file(filename);
    file << "\xEF\xBB\xBF";
    file << rowLabel << "," << columnLabel << ",Count\n";

    for (const auto& [row, columns] : matrix) {
        for (const auto& [column, count] : columns) {
            file << csvEscape(row) << "," << csvEscape(column) << "," << count << "\n";
        }
    }

    if (!includeWideMatrix) {
        return;
    }

    const auto headers = collectMatrixHeaders(matrix);
    file << "\nWideMatrix\n";
    file << csvEscape(rowLabel);
    for (const auto& header : headers) {
        file << "," << csvEscape(header);
    }
    file << "\n";

    for (const auto& row : headers) {
        file << csvEscape(row);
        for (const auto& column : headers) {
            int count = 0;
            const auto rowIt = matrix.find(row);
            if (rowIt != matrix.end()) {
                const auto colIt = rowIt->second.find(column);
                if (colIt != rowIt->second.end()) {
                    count = colIt->second;
                }
            }
            file << "," << count;
        }
        file << "\n";
    }
}

void exportTransitionMatrixCSV(
    const CountMatrix& counts,
    const std::string& filename) {
    std::ofstream file(filename);
    file << "\xEF\xBB\xBF";
    file << "CurrentClass,NextClass,Count,Probability\n";

    const auto probabilities = buildRowNormalizedProbabilityMatrix(counts);
    for (const auto& [row, columns] : counts) {
        for (const auto& [column, count] : columns) {
            const double probability =
                probabilities.count(row) > 0 && probabilities.at(row).count(column) > 0
                    ? probabilities.at(row).at(column)
                    : 0.0;
            file << csvEscape(row) << ","
                 << csvEscape(column) << ","
                 << count << ","
                 << probability << "\n";
        }
    }

    const auto headers = collectMatrixHeaders(counts);
    file << "\nCountWideMatrix\n";
    file << "\"CurrentClass\"";
    for (const auto& header : headers) {
        file << "," << csvEscape(header);
    }
    file << "\n";

    for (const auto& row : headers) {
        file << csvEscape(row);
        for (const auto& column : headers) {
            int count = 0;
            const auto rowIt = counts.find(row);
            if (rowIt != counts.end()) {
                const auto colIt = rowIt->second.find(column);
                if (colIt != rowIt->second.end()) {
                    count = colIt->second;
                }
            }
            file << "," << count;
        }
        file << "\n";
    }

    file << "\nProbabilityWideMatrix\n";
    file << "\"CurrentClass\"";
    for (const auto& header : headers) {
        file << "," << csvEscape(header);
    }
    file << "\n";

    for (const auto& row : headers) {
        file << csvEscape(row);
        for (const auto& column : headers) {
            double probability = 0.0;
            const auto rowIt = probabilities.find(row);
            if (rowIt != probabilities.end()) {
                const auto colIt = rowIt->second.find(column);
                if (colIt != rowIt->second.end()) {
                    probability = colIt->second;
                }
            }
            file << "," << probability;
        }
        file << "\n";
    }
}

void exportSyllableFeatureMatrixCSV(
    const std::vector<SyllableFeatureRow>& rows,
    const std::string& filename) {
    std::ofstream file(filename);
    file << "\xEF\xBB\xBF";
    file << "Verse,WordIndex,SyllableIndex,Syllable,OnsetClass,NucleusBase,CodaClass,Weight,Swara\n";

    for (const auto& row : rows) {
        file << row.verseNumber << ","
             << row.wordIndex << ","
             << row.syllableIndex << ","
             << csvEscape(row.syllable) << ","
             << csvEscape(row.onsetClass) << ","
             << csvEscape(row.nucleusBase) << ","
             << csvEscape(row.codaClass) << ","
             << csvEscape(row.weight) << ","
             << csvEscape(row.swara) << "\n";
    }
}
