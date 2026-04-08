//CleanUp

#include "CleanUp.h"
#include <sstream>

//UTF8 Splitting for Unicode/Grapheme Normalization
std::vector<std::string> splitUTF8(const std::string& s) {
    std::vector<std::string> result;

    for (size_t i = 0; i < s.size();) {
        unsigned char c = static_cast<unsigned char>(s[i]);

        size_t charLen = 0;

        if ((c & 0x80) == 0x00) charLen = 1;          // 1-byte (ASCII)
        else if ((c & 0xE0) == 0xC0) charLen = 2;     // 2-byte
        else if ((c & 0xF0) == 0xE0) charLen = 3;     // 3-byte (most Sanskrit)
        else if ((c & 0xF8) == 0xF0) charLen = 4;     // 4-byte
        else charLen = 1; // fallback

        result.push_back(s.substr(i, charLen));
        i += charLen;
    }

    return result;
}

//Split Words
std::vector<std::string> splitWords(const std::string& line) {
    std::vector<std::string> words;
    std::stringstream ss(line);
    std::string word;

    while (ss >> word) {
        words.push_back(word);
    }

    return words;
}

//Split Lines
std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;

    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    return lines;
}

//Vowel Detection
bool isVowel(const std::string& ch) {
    static const std::vector<std::string> vowels = {
        u8"अ", u8"आ", u8"इ", u8"ई", u8"उ", u8"ऊ",
        u8"ऋ", u8"ॠ", u8"ऌ", u8"ॡ", u8"ए", u8"ऐ", u8"ओ", u8"औ"
    };

    for (const auto& v : vowels) {
        if (ch == v) return true;
    }

    return false;
}

//VowelWeight, if not Long then automatically short.
bool isLongVowel(const std::string& ch) {
    static const std::vector<std::string> longVowels = {
        u8"आ", u8"ई", u8"ऊ", u8"ॠ", u8"ॡ", u8"ए", u8"ऐ", u8"ओ", u8"औ"
    };

    for (const auto& v : longVowels) {
        if (ch == v) return true;
    }

    return false;
}

//Check if IAST char is Vowel
bool isIASTVowel(const std::string& ch) {
    static const std::vector<std::string> vowels = {
        "a", u8"ā", "i", u8"ī", "u", u8"ū", u8"ṛ", u8"ṝ", u8"ḷ", u8"ḹ", "e", "o", "ai", "au"
    };

    for (const auto& v : vowels) {
        if (ch == v) return true;
    }

    return false;
}

//Check if Danda
bool isIgnorableSymbol(const std::string& s) {
    return (
        s == u8"।"  ||
        s == u8"॥" ||
        s == "|"   ||
        s == "||"
    );
}

//CleanWords
std::string cleanWord(const std::string& w) {
    if (isIgnorableSymbol(w)) return "";

    std::string cleaned = w;

    while (!cleaned.empty()) {
        auto chars = splitUTF8(cleaned);
        if (chars.empty()) break;

        const std::string& tail = chars.back();
        if (tail == u8"।" || tail == u8"॥" || tail == "|" || tail == "||") {
            cleaned.erase(cleaned.size() - tail.size());
            continue;
        }

        break;
    }

    return cleaned;
}
