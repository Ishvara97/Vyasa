#include "CleanUp.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace {
const std::unordered_set<std::string>& devVowels() {
    static const std::unordered_set<std::string> values = {
        u8"\u0905", u8"\u0906", u8"\u0907", u8"\u0908", u8"\u0909", u8"\u090a",
        u8"\u090b", u8"\u0960", u8"\u090c", u8"\u0961", u8"\u090f", u8"\u0910", u8"\u0913", u8"\u0914"
    };
    return values;
}

const std::unordered_set<std::string>& devLongVowels() {
    static const std::unordered_set<std::string> values = {
        u8"\u0906", u8"\u0908", u8"\u090a", u8"\u0960", u8"\u0961", u8"\u090f", u8"\u0910", u8"\u0913", u8"\u0914"
    };
    return values;
}

const std::unordered_set<std::string>& iastVowels() {
    static const std::unordered_set<std::string> values = {
        "a", u8"ā", "i", u8"ī", "u", u8"ū", u8"ṛ", u8"ṝ", u8"ḷ", "e", "o", "ai", "au"
    };
    return values;
}

std::string syllableToText(const Syllable& syllable) {
    std::string text = lettersToString(syllable.getOnset());
    text += syllable.getNucleus().getValue();
    text += lettersToString(syllable.getCoda());
    return text;
}
}

std::vector<std::string> splitUTF8(const std::string& s) {
    std::vector<std::string> result;

    for (size_t i = 0; i < s.size();) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        size_t charLen = 1;

        if ((c & 0x80) == 0x00) charLen = 1;
        else if ((c & 0xE0) == 0xC0) charLen = 2;
        else if ((c & 0xF0) == 0xE0) charLen = 3;
        else if ((c & 0xF8) == 0xF0) charLen = 4;

        result.push_back(s.substr(i, charLen));
        i += charLen;
    }

    return result;
}

std::vector<std::string> splitIAST(const std::string& s) {
    std::vector<std::string> result;

    for (size_t i = 0; i < s.size();) {
        if (i + 1 < s.size()) {
            const std::string two = s.substr(i, 2);
            if (two == "ai" || two == "au") {
                result.push_back(two);
                i += 2;
                continue;
            }
        }

        const unsigned char c = static_cast<unsigned char>(s[i]);
        size_t len = 1;

        if ((c & 0x80) == 0x00) len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;

        result.push_back(s.substr(i, len));
        i += len;
    }

    return result;
}

std::vector<std::string> splitWords(const std::string& line) {
    std::vector<std::string> words;
    std::stringstream ss(line);
    std::string word;

    while (ss >> word) {
        words.push_back(word);
    }

    return words;
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;

    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    return lines;
}

bool isVowel(const std::string& ch) {
    return devVowels().find(ch) != devVowels().end();
}

bool isLongVowel(const std::string& ch) {
    return devLongVowels().find(ch) != devLongVowels().end();
}

bool isIASTVowel(const std::string& ch) {
    return iastVowels().find(ch) != iastVowels().end();
}

bool isIgnorableSymbol(const std::string& s) {
    return s == u8"\u0964" || s == u8"\u0965" || s == "|" || s == "||";
}

bool isVirama(const std::string& ch) {
    return ch == u8"\u094d";
}

std::string cleanWord(const std::string& w) {
    if (isIgnorableSymbol(w)) return "";

    std::string cleaned = w;

    while (!cleaned.empty()) {
        auto chars = splitUTF8(cleaned);
        if (chars.empty()) break;

        const std::string& tail = chars.back();
        if (!isIgnorableSymbol(tail)) break;

        cleaned.erase(cleaned.size() - tail.size());
    }

    return cleaned;
}

std::string join(const std::vector<std::string>& vec, const std::string& delim) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i + 1 < vec.size()) {
            oss << delim;
        }
    }
    return oss.str();
}

std::string lettersToString(const std::vector<Letter>& letters) {
    std::string result;
    for (const auto& l : letters) {
        result += l.getValue();
    }
    return result;
}

void exportFullCSV(const Hymn& h, const std::string& filename) {
    std::ofstream file(filename);
    file << "\xEF\xBB\xBF";
    file <<
        "Mandala,Sukta,Rishis,Devatas,Categories,"
        "Verse,Verse_DEV,Verse_IAST,"
        "Word_Index,Word_DEV,Word_IAST,"
        "Syllable_Index,Syllable_DEV,Syllable_IAST,Syllable_Onset,Syllable_Nucleus,Syllable_Coda,"
        "Syllable_Weight,Syllable_Swaras,"
        "Letter_Index,Letter_Value,Letter_HasSwara,Letter_SwaraType\n";

    const std::string rishis = join(h.getRishis(), "|");
    const std::string devatas = join(h.getDevatas(), "|");
    const std::string categories = join(h.getCategories(), "|");

    for (const auto& v : h.getVerses()) {
        const auto& devWords = v.getDevWords();
        const auto& iastWords = v.getIASTWords();

        for (size_t wi = 0; wi < devWords.size(); ++wi) {
            const auto& wDev = devWords[wi];
            const std::string wIAST =
                !wDev.getAlignedIAST().empty()
                    ? wDev.getAlignedIAST()
                    : ((wi < iastWords.size()) ? iastWords[wi].getText() : "");
            const auto& sylls = wDev.getSyllables();
            const auto& alignments = wDev.getAlignment();

            for (size_t si = 0; si < sylls.size(); ++si) {
                const auto& syllable = sylls[si];
                const std::string devSyllable = syllableToText(syllable);
                const std::string iastSyllable =
                    (si < alignments.size()) ? syllableToText(alignments[si].iast) : "";
                const std::string onset = lettersToString(syllable.getOnset());
                const std::string nucleus = syllable.getNucleus().getValue();
                const std::string coda = lettersToString(syllable.getCoda());
                const std::string swaras = join(syllable.getSwaras(), "|");

                std::vector<Letter> allLetters = syllable.getOnset();
                allLetters.push_back(syllable.getNucleus());
                const auto& codaLetters = syllable.getCoda();
                allLetters.insert(allLetters.end(), codaLetters.begin(), codaLetters.end());

                for (size_t li = 0; li < allLetters.size(); ++li) {
                    const auto& letter = allLetters[li];

                    file
                        << h.getMandala() << ","
                        << h.getSukta() << ","
                        << "\"" << rishis << "\","
                        << "\"" << devatas << "\","
                        << "\"" << categories << "\","
                        << v.getVerseNumber() << ","
                        << "\"" << v.getDev() << "\","
                        << "\"" << v.getIAST() << "\","
                        << wi << ","
                        << "\"" << wDev.getText() << "\","
                        << "\"" << wIAST << "\","
                        << si << ","
                        << "\"" << devSyllable << "\","
                        << "\"" << iastSyllable << "\","
                        << "\"" << onset << "\","
                        << "\"" << nucleus << "\","
                        << "\"" << coda << "\","
                        << syllable.getWeight() << ","
                        << "\"" << swaras << "\","
                        << li << ","
                        << "\"" << letter.getValue() << "\","
                        << (letter.getHasSwara() ? "true" : "false") << ",";

                    if (letter.getSwaraType().has_value()) {
                        file << letter.getSwaraType().value();
                    }

                    file << "\n";
                }
            }
        }
    }
}
