#include <map>
#include <vector>
#include <string>
#include <fstream>
#include "PoemStructures.h"

//LetterFreq
std::map<std::string, int> getLetterFrequency(const Verse& v) {
    std::map<std::string, int> freq;

    for (const auto& w : v.getDevWords()) {
        for (const auto& l : w.getLetters()) {
            std::string val = l.getValue();

            // Ignore danda
            if (val == u8"|" || val == u8"॥") continue;

            freq[val]++;
        }
    }

    return freq;
}

//HymnLetterFreq
std::map<std::string, int> getHymnLetterFrequency(const Hymn& h) {
    std::map<std::string, int> freq;

    for (const auto& v : h.getVerses()) {
        for (const auto& w : v.getDevWords()) {
            for (const auto& l : w.getLetters()) {

                std::string val = l.getValue();

                // Ignore danda symbols
                if (val == u8"|" || val == u8"॥") continue;

                freq[val]++;
            }
        }
    }

    return freq;
}

//SwaraFreq
std::map<std::string, int> getSwaraFrequency(const Verse& v) {
    std::map<std::string, int> freq;

    for (const auto& w : v.getDevWords()) {
        for (const auto& l : w.getLetters()) {

            if (l.getSwaraType().has_value()) {
                freq[l.getSwaraType().value()]++;
            }
        }
    }

    return freq;
}

//HymnSwaraFreq
std::map<std::string, int> getHymnSwaraFrequency(const Hymn& h) {
    std::map<std::string, int> freq;

    for (const auto& v : h.getVerses()) {
        for (const auto& w : v.getDevWords()) {
            for (const auto& l : w.getLetters()) {

                if (l.getSwaraType().has_value()) {
                    freq[l.getSwaraType().value()]++;
                }
            }
        }
    }

    return freq;
}
//SyllablePattern
std::vector<std::string> getSyllablePattern(const Verse& v) {
    std::vector<std::string> pattern;

    for (const auto& w : v.getDevWords()) {
        for (const auto& s : w.getSyllables()) {
            pattern.push_back(s.getWeight());
        }
    }

    return pattern;
}

//BaseNGram
std::map<std::string, int> getNGrams(const Verse& v, int n) {
    std::map<std::string, int> freq;

    std::vector<std::string> sequence;

    // Flatten letters
    for (const auto& w : v.getDevWords()) {
        for (const auto& l : w.getLetters()) {
            sequence.push_back(l.getValue());
        }
    }

    for (size_t i = 0; i + n <= sequence.size(); ++i) {
        std::string gram;

        for (int j = 0; j < n; ++j) {
            gram += sequence[i + j];
        }

        freq[gram]++;
    }

    return freq;
}

//ExportAnalysis
void exportHymnAnalysisCSV(const Hymn& h, const std::string& filename) {
    std::ofstream file(filename);
    file << "\xEF\xBB\xBF";
    file << "Type,Key,Value\n";

    auto lf = getHymnLetterFrequency(h);
    for (auto& [k,v] : lf) {
        file << "Letter," << k << "," << v << "\n";
    }

    auto sf = getHymnSwaraFrequency(h);
    for (auto& [k,v] : sf) {
        file << "Swara," << k << "," << v << "\n";
    }
}