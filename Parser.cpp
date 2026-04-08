#include "Parser.h"
#include "CleanUp.h"
#include <fstream>
#include <regex>
#include <vector>

const std::string DEV_SVARITA   = u8"॑";
const std::string DEV_ANUDATTA  = u8"॒";
const std::string COMB_SVARITA  = u8"\u030D";
const std::string COMB_ANUDATTA = u8"\u0331";

namespace {
bool isDevDependentVowel(const std::string& ch) {
    static const std::vector<std::string> vowels = {
        u8"ा", u8"ि", u8"ी", u8"ु", u8"ू", u8"ृ", u8"ॄ",
        u8"ॢ", u8"ॣ", u8"े", u8"ै", u8"ो", u8"ौ"
    };

    for (const auto& v : vowels) {
        if (ch == v) return true;
    }

    return false;
}

bool isDevLongDependentVowel(const std::string& ch) {
    static const std::vector<std::string> longVowels = {
        u8"ा", u8"ी", u8"ू", u8"ॄ", u8"ॣ", u8"े", u8"ै", u8"ो", u8"ौ"
    };

    for (const auto& v : longVowels) {
        if (ch == v) return true;
    }

    return false;
}

bool isVirama(const std::string& ch) {
    return ch == u8"्";
}

bool isDevCodaMark(const std::string& ch) {
    return ch == u8"ं" || ch == u8"ः" || ch == u8"ँ";
}

bool isIASTDiphthongStart(const std::vector<Letter>& letters, size_t index) {
    if (index + 1 >= letters.size()) return false;

    const std::string current = letters[index].getValue();
    const std::string next = letters[index + 1].getValue();

    return current == "a" && (next == "i" || next == "u");
}

std::string cleanTokenForParsing(const std::string& token) {
    std::string cleaned = cleanWord(token);

    while (!cleaned.empty()) {
        auto chars = splitUTF8(cleaned);
        if (chars.empty()) break;

        const std::string& tail = chars.back();
        if (!isIgnorableSymbol(tail)) break;

        cleaned.erase(cleaned.size() - tail.size());
    }

    return cleaned;
}
}

//Syllable Parser
std::vector<Syllable> buildSyllables(const Word& word) {
    std::vector<Syllable> syllables;
    const auto& letters = word.getLetters();

    Syllable current;
    bool hasNucleus = false;

    for (size_t i = 0; i < letters.size(); ++i) {
        const Letter& l = letters[i];
        const std::string val = l.getValue();
        const std::string next = (i + 1 < letters.size()) ? letters[i + 1].getValue() : "";
        const bool nextIsVirama = isVirama(next);
        const bool nextIsDependentVowel = isDevDependentVowel(next);

        if (isVowel(val) || isDevDependentVowel(val)) {
            if (hasNucleus) {
                syllables.push_back(current);
                current = Syllable();
            }

            current.setNucleus(l);
            hasNucleus = true;
            current.setWeight((isLongVowel(val) || isDevLongDependentVowel(val)) ? "heavy" : "light");

            if (l.getSwaraType().has_value()) {
                current.addSwara(l.getSwaraType().value());
            }
            continue;
        }

        if (isVirama(val)) {
            if (!hasNucleus) {
                current.addOnset(l);
            }
            continue;
        }

        if (isDevCodaMark(val)) {
            if (!hasNucleus) {
                current.setNucleus(l);
                hasNucleus = true;
            } else {
                current.addCoda(l);
            }
            current.setWeight("heavy");
            continue;
        }

        if (!hasNucleus) {
            if (!nextIsVirama && !nextIsDependentVowel) {
                current.setNucleus(l);
                hasNucleus = true;
                current.setWeight("light");
                if (l.getSwaraType().has_value()) {
                    current.addSwara(l.getSwaraType().value());
                }
            } else {
                current.addOnset(l);
            }
        } else {
            current.addCoda(l);
            current.setWeight("heavy");
        }
    }

    if (hasNucleus) {
        if (!current.getCoda().empty()) {
            current.setWeight("heavy");
        }
        syllables.push_back(current);
    }

    return syllables;
}

//Dev WordBuilder w/ Swaras
Word buildWordDEV(const std::string& raw) {
    Word w(raw);
    auto chars = splitUTF8(raw);

    Letter currentLetter("");
    bool hasPending = false;

    for (const auto& ch : chars) {
        if (ch == DEV_SVARITA || ch == COMB_SVARITA || ch == DEV_ANUDATTA || ch == COMB_ANUDATTA) {
            if (hasPending) {
                if (ch == DEV_SVARITA || ch == COMB_SVARITA) {
                    currentLetter.setSwara(true, "svarita");
                } else {
                    currentLetter.setSwara(true, "anudatta");
                }
            }
            continue;
        }

        if (hasPending) {
            if (!currentLetter.getHasSwara()) {
                currentLetter.setSwara(false, "udatta");
            }
            w.addLetter(currentLetter);
        }

        currentLetter = Letter(ch);
        hasPending = true;
    }

    if (hasPending) {
        if (!currentLetter.getHasSwara()) {
            currentLetter.setSwara(false, "udatta");
        }
        w.addLetter(currentLetter);
    }

    auto sylls = buildSyllables(w);
    for (const auto& s : sylls) {
        w.addSyllable(s);
    }

    return w;
}

//Build IAST Syllables
std::vector<Syllable> buildSyllablesIAST(const Word& word) {
    std::vector<Syllable> syllables;
    const auto& letters = word.getLetters();

    Syllable current;
    bool hasNucleus = false;

    for (size_t i = 0; i < letters.size(); ++i) {
        const Letter& l = letters[i];
        std::string val = l.getValue();

        if (isIASTDiphthongStart(letters, i)) {
            val += letters[i + 1].getValue();
            ++i;
        }

        if (isIASTVowel(val)) {
            if (hasNucleus) {
                syllables.push_back(current);
                current = Syllable();
            }

            current.setNucleus(Letter(val));
            hasNucleus = true;
        } else {
            if (!hasNucleus) {
                current.addOnset(l);
            } else {
                current.addCoda(l);
            }
        }
    }

    if (hasNucleus) {
        syllables.push_back(current);
    }

    return syllables;
}

//WordBuilder IAST (No SwaraChecks)
Word buildWordIAST(const std::string& raw) {
    Word w(raw);
    auto chars = splitUTF8(raw);

    for (const auto& ch : chars) {
        Letter l(ch);
        l.setSwara(false, std::nullopt);
        w.addLetter(l);
    }

    auto sylls = buildSyllablesIAST(w);
    for (const auto& s : sylls) {
        w.addIASTSyllable(s);
    }

    return w;
}

//AlignWords
void alignWord(Word& devWord, Word& iastWord) {
    devWord.clearAlignment();

    const auto& devSylls = devWord.getSyllables();
    const auto& iastSylls = iastWord.getIASTSyllables();
    const size_t n = std::min(devSylls.size(), iastSylls.size());

    for (size_t i = 0; i < n; ++i) {
        SyllableAlignment a;
        a.dev = devSylls[i];
        a.iast = iastSylls[i];
        devWord.addAlignment(a);
    }
}

//Hymn Parse
Hymn parseHymn(const std::string& filename) {
    Hymn hymn;

    std::ifstream file(filename);
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto lines = splitLines(text);

    Verse currentVerse;
    std::regex mandala_re("Mandala:\\s*(\\d+)");
    std::regex sukta_re("Sukta:\\s*(\\d+)");
    std::regex rishi_re("Rishis:\\s*(.*)");
    std::regex devata_re("Devatas:\\s*(.*)");
    std::regex category_re("Categories:\\s*(.*)");
    std::regex verse_re("\\[Verse\\s*(\\d+)\\]");

    std::smatch match;
    for (const auto& line : lines) {
        if (std::regex_search(line, match, mandala_re)) {
            hymn.setMandala(std::stoi(match[1]));
        }
        else if (std::regex_search(line, match, sukta_re)) {
            hymn.setSukta(std::stoi(match[1]));
        }
        else if (std::regex_search(line, match, rishi_re)) {
            hymn.addRishi(match[1]);
        }
        else if (std::regex_search(line, match, devata_re)) {
            hymn.addDevata(match[1]);
        }
        else if (std::regex_search(line, match, category_re)) {
            hymn.addCategory(match[1]);
        }
        else if (std::regex_search(line, match, verse_re)) {
            if (currentVerse.getVerseNumber() != 0) {
                auto& devWords = currentVerse.getDevWordsMutable();
                auto& iastWords = currentVerse.getIASTWordsMutable();
                const size_t n = std::min(devWords.size(), iastWords.size());

                for (size_t i = 0; i < n; ++i) {
                    alignWord(devWords[i], iastWords[i]);
                    devWords[i].setAlignedIAST(iastWords[i].getText());
                }

                hymn.addVerse(currentVerse);
                currentVerse = Verse();
            }

            currentVerse.setVerseNumber(std::stoi(match[1]));
        }
        else if (line.rfind("DEV:", 0) == 0) {
            std::string devLine = line.substr(4);
            currentVerse.setDev(devLine);

            auto devWords = splitWords(devLine);
            for (const auto& w : devWords) {
                auto cleaned = cleanTokenForParsing(w);
                if (cleaned.empty()) continue;
                currentVerse.addDevWord(buildWordDEV(cleaned));
            }
        }
        else if (line.rfind("IAST:", 0) == 0) {
            std::string iastLine = line.substr(5);
            currentVerse.setIAST(iastLine);

            auto iastWords = splitWords(iastLine);
            for (const auto& w : iastWords) {
                auto cleaned = cleanTokenForParsing(w);
                if (cleaned.empty()) continue;
                currentVerse.addIASTWord(buildWordIAST(cleaned));
            }
        }
        else if (line.rfind("ENG:", 0) == 0) {
            currentVerse.setENG(line.substr(4));
        }
    }

    if (currentVerse.getVerseNumber() != 0) {
        auto& devWords = currentVerse.getDevWordsMutable();
        auto& iastWords = currentVerse.getIASTWordsMutable();
        const size_t n = std::min(devWords.size(), iastWords.size());

        for (size_t i = 0; i < n; ++i) {
            alignWord(devWords[i], iastWords[i]);
            devWords[i].setAlignedIAST(iastWords[i].getText());
        }

        hymn.addVerse(currentVerse);
    }

    return hymn;
}
