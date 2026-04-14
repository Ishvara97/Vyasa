#include "Sandhi.h"
#include "CleanUp.h"
#include <map>
#include <sstream>

namespace {
bool isCombiningOrIgnorableIAST(const std::string& unit) {
    return unit == u8"\u030D" || unit == u8"\u0331" || isIgnorableSymbol(unit);
}

std::vector<std::string> splitIASTUnitsForSandhi(const std::string& text) {
    std::vector<std::string> raw = splitIAST(text);
    std::vector<std::string> units;

    for (size_t i = 0; i < raw.size(); ++i) {
        if (isCombiningOrIgnorableIAST(raw[i])) {
            continue;
        }

        if (i + 1 < raw.size()) {
            const std::string pair = raw[i] + raw[i + 1];
            if (pair == "kh" || pair == "gh" || pair == "ch" || pair == "jh" ||
                pair == "th" || pair == "dh" || pair == "ph" || pair == "bh") {
                units.push_back(pair);
                ++i;
                continue;
            }
        }

        if (i + 2 < raw.size()) {
            const std::string triplet = raw[i] + raw[i + 1] + raw[i + 2];
            if (triplet == u8"ṭh" || triplet == u8"ḍh") {
                units.push_back(triplet);
                i += 2;
                continue;
            }
        }

        units.push_back(raw[i]);
    }

    return units;
}

bool isIASTVowelUnit(const std::string& unit) {
    return unit == "a" || unit == u8"ā" || unit == "i" || unit == u8"ī" ||
           unit == "u" || unit == u8"ū" || unit == u8"ṛ" || unit == u8"ṝ" ||
           unit == u8"ḷ" || unit == u8"ḹ" || unit == "e" || unit == "o" ||
           unit == "ai" || unit == "au";
}

bool isIASTVowelLike(const std::string& unit) {
    return isIASTVowelUnit(unit);
}

bool isSavarnaPair(const std::string& left, const std::string& right) {
    return (left == "a" && right == "a") ||
           (left == u8"ā" && right == "a") ||
           (left == "a" && right == u8"ā") ||
           (left == u8"ā" && right == u8"ā") ||
           (left == "i" && right == "i") ||
           (left == u8"ī" && right == "i") ||
           (left == "i" && right == u8"ī") ||
           (left == u8"ī" && right == u8"ī") ||
           (left == "u" && right == "u") ||
           (left == u8"ū" && right == "u") ||
           (left == "u" && right == u8"ū") ||
           (left == u8"ū" && right == u8"ū") ||
           (left == u8"ṛ" && right == u8"ṛ") ||
           (left == u8"ṝ" && right == u8"ṛ") ||
           (left == u8"ṛ" && right == u8"ṝ") ||
           (left == u8"ṝ" && right == u8"ṝ") ||
           (left == u8"ḷ" && right == u8"ḷ");
}

bool canSemivowelShift(const std::string& left) {
    return left == "i" || left == u8"ī" ||
           left == "u" || left == u8"ū" ||
           left == u8"ṛ" || left == u8"ṝ" ||
           left == u8"ḷ";
}

std::string firstIASTUnit(const std::string& token) {
    const auto units = splitIASTUnitsForSandhi(token);
    for (const auto& unit : units) {
        if (unit == "'" || unit == u8"’") {
            continue;
        }
        return unit;
    }
    return "";
}

std::string lastIASTUnit(const std::string& token) {
    const auto units = splitIASTUnitsForSandhi(token);
    for (size_t i = units.size(); i > 0; --i) {
        const std::string& unit = units[i - 1];
        if (unit == "'" || unit == u8"’") {
            continue;
        }
        return unit;
    }
    return "";
}

std::string mapIndependentVowelToDEV(const std::string& vowel) {
    static const std::map<std::string, std::string> vowels = {
        {"a", u8"अ"}, {u8"ā", u8"आ"}, {"i", u8"इ"}, {u8"ī", u8"ई"},
        {"u", u8"उ"}, {u8"ū", u8"ऊ"}, {u8"ṛ", u8"ऋ"}, {u8"ṝ", u8"ॠ"},
        {u8"ḷ", u8"ऌ"}, {u8"ḹ", u8"ॡ"}, {"e", u8"ए"}, {"o", u8"ओ"},
        {"ai", u8"ऐ"}, {"au", u8"औ"}
    };

    auto it = vowels.find(vowel);
    return it != vowels.end() ? it->second : vowel;
}

std::string mapDependentVowelToDEV(const std::string& vowel) {
    static const std::map<std::string, std::string> vowels = {
        {"a", ""}, {u8"ā", u8"ा"}, {"i", u8"ि"}, {u8"ī", u8"ी"},
        {"u", u8"ु"}, {u8"ū", u8"ू"}, {u8"ṛ", u8"ृ"}, {u8"ṝ", u8"ॄ"},
        {u8"ḷ", u8"ॢ"}, {u8"ḹ", u8"ॣ"}, {"e", u8"े"}, {"o", u8"ो"},
        {"ai", u8"ै"}, {"au", u8"ौ"}
    };

    auto it = vowels.find(vowel);
    return it != vowels.end() ? it->second : "";
}

std::string mapConsonantToDEV(const std::string& consonant) {
    static const std::map<std::string, std::string> consonants = {
        {"k", u8"क"}, {"kh", u8"ख"}, {"g", u8"ग"}, {"gh", u8"घ"}, {u8"ṅ", u8"ङ"},
        {"c", u8"च"}, {"ch", u8"छ"}, {"j", u8"ज"}, {"jh", u8"झ"}, {u8"ñ", u8"ञ"},
        {u8"ṭ", u8"ट"}, {u8"ṭh", u8"ठ"}, {u8"ḍ", u8"ड"}, {u8"ḍh", u8"ढ"}, {u8"ṇ", u8"ण"},
        {"t", u8"त"}, {"th", u8"थ"}, {"d", u8"द"}, {"dh", u8"ध"}, {"n", u8"न"},
        {"p", u8"प"}, {"ph", u8"फ"}, {"b", u8"ब"}, {"bh", u8"भ"}, {"m", u8"म"},
        {"y", u8"य"}, {"r", u8"र"}, {"l", u8"ल"}, {"v", u8"व"},
        {u8"ś", u8"श"}, {u8"ṣ", u8"ष"}, {"s", u8"स"}, {"h", u8"ह"}
    };

    auto it = consonants.find(consonant);
    return it != consonants.end() ? it->second : consonant;
}

std::string joinUnderlyingDev(const std::vector<Word>& words, size_t start, size_t end) {
    std::ostringstream out;
    for (size_t i = start; i < end; ++i) {
        if (i > start) {
            out << " ";
        }
        if (!words[i].getUnderlyingDevText().empty()) {
            out << words[i].getUnderlyingDevText();
        } else {
            out << transliterateIASTToDEV(words[i].getUnderlyingText());
        }
    }
    return out.str();
}
}

std::string transliterateIASTToDEV(const std::string& text) {
    std::ostringstream out;
    const auto tokens = splitWords(text);

    for (size_t tokenIndex = 0; tokenIndex < tokens.size(); ++tokenIndex) {
        if (tokenIndex > 0) {
            out << " ";
        }

        const auto units = splitIASTUnitsForSandhi(tokens[tokenIndex]);
        for (size_t i = 0; i < units.size(); ++i) {
            const std::string& unit = units[i];

            if (unit == "'" || unit == u8"’") {
                out << u8"ऽ";
                continue;
            }

            if (unit == u8"ṃ" || unit == u8"ṁ") {
                out << u8"ं";
                continue;
            }

            if (unit == u8"ḥ") {
                out << u8"ः";
                continue;
            }

            if (isIASTVowelUnit(unit)) {
                out << mapIndependentVowelToDEV(unit);
                continue;
            }

            const std::string consonant = mapConsonantToDEV(unit);
            const bool hasNext = i + 1 < units.size();
            const std::string next = hasNext ? units[i + 1] : "";

            if (hasNext && isIASTVowelUnit(next)) {
                out << consonant << mapDependentVowelToDEV(next);
                ++i;
            } else {
                out << consonant;
                if (hasNext && next != u8"ṃ" && next != u8"ṁ" && next != u8"ḥ" &&
                    next != "'" && next != u8"’") {
                    out << u8"्";
                }
            }
        }
    }

    return out.str();
}

SandhiBoundary buildSandhiBoundary(
    const Word& surfaceWord,
    const std::vector<Word>& underlyingWords,
    size_t start,
    size_t leftIndex,
    size_t rightIndex) {
    SandhiBoundary boundary;
    boundary.surfaceWordIndex = static_cast<int>(start);
    boundary.leftUnderlyingWordIndex = static_cast<int>(leftIndex);
    boundary.rightUnderlyingWordIndex = static_cast<int>(rightIndex);
    boundary.surfaceDev = surfaceWord.getText();
    boundary.surfaceIAST = surfaceWord.getAlignedIAST();
    boundary.segmentedIAST = surfaceWord.getAlignedIASTUnderlying();
    boundary.segmentedDev = surfaceWord.getUnderlyingDevText();
    boundary.leftUnderlyingIAST = underlyingWords[leftIndex].getUnderlyingText();
    boundary.rightUnderlyingIAST = underlyingWords[rightIndex].getUnderlyingText();
    boundary.leftUnderlyingDev = !underlyingWords[leftIndex].getUnderlyingDevText().empty()
        ? underlyingWords[leftIndex].getUnderlyingDevText()
        : transliterateIASTToDEV(boundary.leftUnderlyingIAST);
    boundary.rightUnderlyingDev = !underlyingWords[rightIndex].getUnderlyingDevText().empty()
        ? underlyingWords[rightIndex].getUnderlyingDevText()
        : transliterateIASTToDEV(boundary.rightUnderlyingIAST);
    boundary.detected = true;
    boundary.confidence = "medium";
    boundary.normalizationStatus = "generated";

    const std::string leftUnit = lastIASTUnit(boundary.leftUnderlyingIAST);
    const std::string rightUnit = firstIASTUnit(boundary.rightUnderlyingIAST);

    if (leftUnit == u8"ḥ") {
        boundary.category = "visarga sandhi";
        boundary.subtype = "external visarga";
        boundary.confidence = "high";
        boundary.notes = "Left underlying form ends in visarga before the next segment.";
        return boundary;
    }

    if (isIASTVowelLike(leftUnit) && isIASTVowelLike(rightUnit)) {
        if (isSavarnaPair(leftUnit, rightUnit)) {
            boundary.category = "vowel sandhi";
            boundary.subtype = "savarna dirgha";
            boundary.confidence = "high";
            boundary.notes = "Adjacent vowels are homorganic and may contract to a long vowel.";
            return boundary;
        }

        if (canSemivowelShift(leftUnit)) {
            boundary.category = "semivowel sandhi";
            boundary.subtype = "external glide formation";
            boundary.notes = "A final high vowel before another vowel may surface as y, v, r, or l.";
            return boundary;
        }

        boundary.category = "vowel sandhi";
        boundary.subtype = "heterogeneous vowel contact";
        boundary.notes = "Adjacent vowels across the boundary suggest vowel sandhi.";
        return boundary;
    }

    if (!leftUnit.empty() && !rightUnit.empty() &&
        !isIASTVowelLike(leftUnit) && !isIASTVowelLike(rightUnit)) {
        boundary.category = "consonant sandhi";
        boundary.subtype = "external consonant contact";
        boundary.notes = "Both sides of the boundary are consonantal.";
        return boundary;
    }

    boundary.category = "sandhi candidate";
    boundary.subtype = "boundary assimilation";
    boundary.notes = "Surface token aligns with multiple underlying words.";
    return boundary;
}

std::vector<std::string> getNormalizedIASTWords(const Verse& v) {
    std::vector<std::string> normalized;

    for (const auto& word : v.getIASTWords()) {
        const std::string source = !word.getUnderlyingText().empty()
            ? word.getUnderlyingText()
            : word.getText();

        if (source.empty()) {
            continue;
        }

        for (const auto& token : splitWords(source)) {
            const std::string cleaned = cleanWord(token);
            if (!cleaned.empty()) {
                normalized.push_back(cleaned);
            }
        }
    }

    if (!normalized.empty()) {
        return normalized;
    }

    for (const auto& word : v.getDevWords()) {
        const std::string source = !word.getAlignedIASTUnderlying().empty()
            ? word.getAlignedIASTUnderlying()
            : word.getAlignedIAST();

        if (source.empty()) {
            continue;
        }

        for (const auto& token : splitWords(source)) {
            const std::string cleaned = cleanWord(token);
            if (!cleaned.empty()) {
                normalized.push_back(cleaned);
            }
        }
    }

    return normalized;
}
