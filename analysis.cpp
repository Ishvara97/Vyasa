#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "analysis.h"
#include "CleanUp.h"
#include "Parser.h"
#include "PoemStructures.h"

// Count raw Devanagari letters per verse.
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

// Aggregate verse-level letter counts across the whole hymn.
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

// Count named swara annotations on DEV letters.
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

// Aggregate swara counts across the hymn.
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
// Preserve the verse meter shape as a sequence of light/heavy values.
std::vector<std::string> getSyllablePattern(const Verse& v) {
    std::vector<std::string> pattern;

    for (const auto& w : v.getDevWords()) {
        for (const auto& s : w.getSyllables()) {
            pattern.push_back(s.getWeight());
        }
    }

    return pattern;
}

// Build n-grams over the flattened DEV letter stream.
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

// Count letters by high-level phoneme class.
std::map<std::string, int> getPhonemeClassFrequency(const Verse& v) {
    std::map<std::string, int> freq;

    for (const auto& w : v.getDevWords()) {
        for (const auto& l : w.getLetters()) {
            const std::string phonemeClass = phonemeClassToString(l.getPhoneme());
            if (phonemeClass != "Other") {
                freq[phonemeClass]++;
            }
        }
    }

    return freq;
}

std::map<std::string, int> getHymnPhonemeClassFrequency(const Hymn& h) {
    std::map<std::string, int> freq;

    for (const auto& verse : h.getVerses()) {
        const auto verseFreq = getPhonemeClassFrequency(verse);
        for (const auto& [key, value] : verseFreq) {
            freq[key] += value;
        }
    }

    return freq;
}
// Return the words that contain at least one consonant from the requested class.
std::vector<std::string> findWordsWithClass(const Verse& v, ConsonantClass cls) {
    std::vector<std::string> result;

    for (const auto& w : v.getDevWords()) {

        for (const auto& l : w.getLetters()) {
            if (l.getPhoneme().consonantClass == cls) {
                result.push_back(w.getText());
                break;
            }
        }
    }

    return result;
}

// Convert consonant classes into a compact symbolic sequence for later pattern work.
std::vector<std::string> getConsonantClassSequence(const Verse& v) {
    std::vector<std::string> seq;

    for (const auto& w : v.getDevWords()) {
        for (const auto& l : w.getLetters()) {

            auto cls = l.getPhoneme().consonantClass;

            switch (cls) {
                case ConsonantClass::Velar: seq.push_back("V"); break;
                case ConsonantClass::Palatal: seq.push_back("P"); break;
                case ConsonantClass::Retroflex: seq.push_back("R"); break;
                case ConsonantClass::Dental: seq.push_back("D"); break;
                case ConsonantClass::Labial: seq.push_back("L"); break;
                case ConsonantClass::Nasal: seq.push_back("N"); break;
                case ConsonantClass::Semivowel: seq.push_back("S"); break;
                case ConsonantClass::Liquid: seq.push_back("Q"); break;
                case ConsonantClass::Sibilant: seq.push_back("X"); break;
                case ConsonantClass::Aspirate: seq.push_back("H"); break;
                default: break;
            }
        }
    }

    return seq;
}
// Export hymn-level summaries as a flat CSV for spreadsheet analysis.
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

    auto pf = getHymnPhonemeClassFrequency(h);
    for (auto& [k, v] : pf) {
        file << "PhonemeClass," << k << "," << v << "\n";
    }
}

//Split Verse into Padas
std::vector<std::string> splitVerseIntoPadas(const std::string& text) {
    std::vector<std::string> padas;
    std::string current;

    auto chars = splitUTF8(text);

    for (const auto& ch : chars) {
        if (ch == "|" || ch == u8"।" || ch == u8"॥") {
            if (!current.empty()) {
                padas.push_back(current);
                current.clear();
            }
        } else {
            current += ch;
        }
    }

    if (!current.empty()) {
        padas.push_back(current);
    }

    return padas;
}

//Count Syllables in a Pada
int countSyllablesInPada(const std::string& pada) {
    int count = 0;

    auto words = splitWords(pada);

    for (const auto& w : words) {
        Word tempWord = buildWordDEV(w);
        count += static_cast<int>(tempWord.getSyllables().size());
    }

    return count;
}

//Syllable Counts Per Pada
std::vector<int> getPadaSyllableCounts(const Verse& v) {
    std::vector<int> counts;

    auto padas = splitVerseIntoPadas(v.getDev());

    for (const auto& pada : padas) {
        counts.push_back(countSyllablesInPada(pada));
    }

    return counts;
}
//Format Counts
std::string formatPadaCounts(const std::vector<int>& counts) {
    std::ostringstream oss;

    for (size_t i = 0; i < counts.size(); ++i) {
        if (i > 0) oss << "-";
        oss << counts[i];
    }

    return oss.str();
}

//Meter Detection
std::string detectVerseMeter(const Verse& v) {
    auto counts = getPadaSyllableCounts(v);

    if (counts.empty()) {
        return "No meter detected";
    }

    // Common Vedic meters by pada syllable counts

    if (counts.size() == 3 &&
        counts[0] == 8 && counts[1] == 8 && counts[2] == 8) {
        return "Gayatri";
    }

    if (counts.size() == 4 &&
        counts[0] == 8 && counts[1] == 8 && counts[2] == 8 && counts[3] == 8) {
        return "Anustubh";
    }

    if (counts.size() == 4 &&
        counts[0] == 8 && counts[1] == 8 && counts[2] == 8 && counts[3] == 12) {
        return "Brhati";
    }

    if (counts.size() == 5 &&
        counts[0] == 8 && counts[1] == 8 && counts[2] == 8 &&
        counts[3] == 8 && counts[4] == 8) {
        return "Pankti";
    }

    if (counts.size() == 4 &&
        counts[0] == 11 && counts[1] == 11 && counts[2] == 11 && counts[3] == 11) {
        return "Tristubh";
    }

    if (counts.size() == 4 &&
        counts[0] == 12 && counts[1] == 12 && counts[2] == 12 && counts[3] == 12) {
        return "Jagati";
    }

    if (counts.size() == 4 &&
        counts[0] == 8 && counts[1] == 8 && counts[2] == 12 && counts[3] == 8) {
        return "Ushnih-like";
    }

    return "No meter detected";
}