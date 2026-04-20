#include <map>
#include <vector>
#include <string>
#include <functional>
#include <sstream>
#include <fstream>
#include "analysis.h"
#include "CleanUp.h"
#include "Parser.h"
#include "Sandhi.h"
#include "PoemStructures.h"

namespace {
struct NamedMeterPattern {
    const char* name;
    std::vector<int> counts;
};

bool isDevDependentVowelForMeter(const std::string& ch) {
    static const std::vector<std::string> vowels = {
        u8"\u093e", u8"\u093f", u8"\u0940", u8"\u0941", u8"\u0942", u8"\u0943", u8"\u0944",
        u8"\u0962", u8"\u0963", u8"\u0947", u8"\u0948", u8"\u094b", u8"\u094c"
    };

    for (const auto& vowel : vowels) {
        if (ch == vowel) {
            return true;
        }
    }

    return false;
}

bool isDevCodaMarkForMeter(const std::string& ch) {
    return ch == u8"\u0902" || ch == u8"\u0903" || ch == u8"\u0901";
}

bool isCountableDevConsonantForMeter(const std::string& ch) {
    return !ch.empty() &&
           !isVowel(ch) &&
           !isDevDependentVowelForMeter(ch) &&
           !isVirama(ch) &&
           !isDevCodaMarkForMeter(ch) &&
           !isIgnorableSymbol(ch) &&
           ch != u8"\u093d";
}

std::vector<std::string> splitVerseIntoPadasNormalized(const std::string& text) {
    std::vector<std::string> padas;
    std::string current;

    for (const auto& ch : splitUTF8(text)) {
        if (isIgnorableSymbol(ch)) {
            const std::string cleaned = cleanWord(current);
            if (!cleaned.empty()) {
                padas.push_back(cleaned);
            }
            current.clear();
            continue;
        }

        current += ch;
    }

    const std::string cleaned = cleanWord(current);
    if (!cleaned.empty()) {
        padas.push_back(cleaned);
    }

    return padas;
}

int countSyllablesInPadaNormalized(const std::string& pada) {
    int count = 0;

    for (const auto& token : splitWords(pada)) {
        const std::string cleaned = cleanWord(token);
        if (cleaned.empty()) {
            continue;
        }

        const auto letters = splitUTF8(cleaned);
        for (size_t i = 0; i < letters.size(); ++i) {
            const std::string& ch = letters[i];
            if (isVowel(ch)) {
                ++count;
                continue;
            }

            if (!isCountableDevConsonantForMeter(ch)) {
                continue;
            }

            const bool nextIsVirama =
                (i + 1 < letters.size()) && isVirama(letters[i + 1]);

            if (!nextIsVirama) {
                ++count;
            }
        }
    }

    return count;
}

std::vector<std::string> splitTextIntoPadasNormalized(const std::string& text) {
    return splitVerseIntoPadasNormalized(text);
}

int countIASTSyllablesInPadaNormalized(const std::string& pada) {
    int count = 0;

    for (const auto& token : splitWords(pada)) {
        const std::string cleaned = cleanWord(token);
        if (cleaned.empty()) {
            continue;
        }

        count += static_cast<int>(buildWordIAST(cleaned).getIASTSyllables().size());
    }

    return count;
}

std::vector<int> getExplicitIASTPadaCounts(const Verse& v) {
    std::vector<int> counts;

    for (const auto& pada : splitTextIntoPadasNormalized(v.getIAST())) {
        counts.push_back(countIASTSyllablesInPadaNormalized(pada));
    }

    return counts;
}

const std::vector<NamedMeterPattern>& standardMeterPatterns() {
    static const std::vector<NamedMeterPattern> patterns = {
        {"Gayatri", {8, 8, 8}},
        {"Anustubh", {8, 8, 8, 8}},
        {"Brhati", {8, 8, 8, 12}},
        {"Pankti", {8, 8, 8, 8, 8}},
        {"Tristubh", {11, 11, 11, 11}},
        {"Jagati", {12, 12, 12, 12}},
        {"Ushnih-like", {8, 8, 12, 8}}
    };

    return patterns;
}

std::optional<std::vector<int>> inferStandardPadaCountsFromIASTWords(const Verse& v) {
    std::vector<int> wordCounts;
    std::vector<bool> reducibleWordCounts;

    for (const auto& token : getNormalizedIASTWords(v)) {
        if (token.empty()) {
            continue;
        }

        const auto syllableCount = static_cast<int>(buildWordIAST(token).getIASTSyllables().size());
        if (syllableCount > 0) {
            wordCounts.push_back(syllableCount);
            reducibleWordCounts.push_back(token.back() == 'a' &&
                                          syllableCount > 1);
        }
    }

    if (wordCounts.empty()) {
        return std::nullopt;
    }

    int total = 0;
    for (int count : wordCounts) {
        total += count;
    }

    for (const auto& pattern : standardMeterPatterns()) {
        int targetTotal = 0;
        for (int count : pattern.counts) {
            targetTotal += count;
        }

        int reducibleCount = 0;
        for (bool reducible : reducibleWordCounts) {
            if (reducible) {
                ++reducibleCount;
            }
        }

        if (targetTotal > total || targetTotal < total - reducibleCount) {
            continue;
        }

        std::function<bool(size_t, size_t, int)> matchesPattern =
            [&](size_t wordIndex, size_t padaIndex, int running) -> bool {
                if (padaIndex == pattern.counts.size()) {
                    return wordIndex == wordCounts.size();
                }

                if (running == pattern.counts[padaIndex]) {
                    return matchesPattern(wordIndex, padaIndex + 1, 0);
                }

                if (wordIndex >= wordCounts.size() || running > pattern.counts[padaIndex]) {
                    return false;
                }

                if (matchesPattern(wordIndex + 1, padaIndex, running + wordCounts[wordIndex])) {
                    return true;
                }

                if (reducibleWordCounts[wordIndex] &&
                    matchesPattern(wordIndex + 1, padaIndex, running + wordCounts[wordIndex] - 1)) {
                    return true;
                }

                return false;
            };

        if (matchesPattern(0, 0, 0)) {
            return pattern.counts;
        }
    }

    return std::nullopt;
}

bool allCountsInSet(const std::vector<int>& counts, const std::vector<int>& allowed) {
    for (int count : counts) {
        bool present = false;
        for (int allowedValue : allowed) {
            if (count == allowedValue) {
                present = true;
                break;
            }
        }

        if (!present) {
            return false;
        }
    }

    return !counts.empty();
}

bool containsCount(const std::vector<int>& counts, int target) {
    for (int count : counts) {
        if (count == target) {
            return true;
        }
    }

    return false;
}
}

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
    return countSyllablesInPadaNormalized(pada);
}

//Syllable Counts Per Pada
std::vector<int> getPadaSyllableCounts(const Verse& v) {
    const auto explicitIASTCounts = getExplicitIASTPadaCounts(v);
    if (explicitIASTCounts.size() >= 3) {
        return explicitIASTCounts;
    }

    if (auto inferredCounts = inferStandardPadaCountsFromIASTWords(v); inferredCounts.has_value()) {
        return inferredCounts.value();
    }

    if (!explicitIASTCounts.empty()) {
        return explicitIASTCounts;
    }

    std::vector<int> counts;
    auto padas = splitVerseIntoPadasNormalized(v.getDev());

    for (const auto& pada : padas) {
        counts.push_back(countSyllablesInPadaNormalized(pada));
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
        return "No Meter Detected";
    }

    for (const auto& pattern : standardMeterPatterns()) {
        if (counts == pattern.counts) {
            return pattern.name;
        }
    }

    if (counts.size() == 4 && allCountsInSet(counts, {10, 11}) && containsCount(counts, 10)) {
        return "Nicrd Tristubh";
    }

    if (counts.size() == 4 && allCountsInSet(counts, {11, 12}) &&
        containsCount(counts, 11) && containsCount(counts, 12)) {
        return "Tristubh-Jagati Mixed";
    }

    if (counts.size() == 3 && allCountsInSet(counts, {7, 8}) && containsCount(counts, 7)) {
        return "Nicrd Gayatri";
    }

    if (counts.size() == 4 && allCountsInSet(counts, {7, 8}) && containsCount(counts, 7)) {
        return "Nicrd Anustubh";
    }

    return "No Meter Detected";
}
