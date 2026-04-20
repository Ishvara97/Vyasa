#include "Parser.h"
#include "CleanUp.h"
#include "Sandhi.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <vector>

const std::string DEV_SVARITA = u8"\u0951";
const std::string DEV_ANUDATTA = u8"\u0952";
const std::string COMB_SVARITA = u8"\u030D";
const std::string COMB_ANUDATTA = u8"\u0331";

Word buildWordIAST(const std::string& raw);
void alignWord(Word& devWord, Word& iastWord);

namespace {
// Identify Devanagari vowel signs that attach to a consonant nucleus.
bool isDevDependentVowel(const std::string& ch) {
    static const std::vector<std::string> vowels = {
        u8"\u093e", u8"\u093f", u8"\u0940", u8"\u0941", u8"\u0942", u8"\u0943", u8"\u0944",
        u8"\u0962", u8"\u0963", u8"\u0947", u8"\u0948", u8"\u094b", u8"\u094c"
    };

    for (const auto& v : vowels) {
        if (ch == v) return true;
    }

    return false;
}

bool isDevLongDependentVowel(const std::string& ch) {
    static const std::vector<std::string> longVowels = {
        u8"\u093e", u8"\u0940", u8"\u0942", u8"\u0944", u8"\u0963", u8"\u0947", u8"\u0948", u8"\u094b", u8"\u094c"
    };

    for (const auto& v : longVowels) {
        if (ch == v) return true;
    }

    return false;
}

bool isDevCodaMark(const std::string& ch) {
    return ch == u8"\u0902" || ch == u8"\u0903" || ch == u8"\u0901";
}

bool isIASTCombiningMark(const std::string& ch) {
    return ch == COMB_SVARITA || ch == COMB_ANUDATTA;
}

bool isIASTCodaMark(const std::string& ch) {
    return ch == u8"ṃ" || ch == u8"ṁ" || ch == u8"ḥ";
}

bool isDevConsonant(const std::string& ch) {
    return !ch.empty() &&
           !isVowel(ch) &&
           !isDevDependentVowel(ch) &&
           !isVirama(ch) &&
           !isDevCodaMark(ch) &&
           !isIgnorableSymbol(ch);
}

bool isIASTLongVowelValue(const std::string& ch) {
    return ch == u8"ā" || ch == u8"ī" || ch == u8"ū" ||
           ch == u8"e" || ch == u8"o" || ch == "ai" || ch == "au";
}

bool isIASTConsonant(const std::string& ch) {
    return !ch.empty() &&
           !isIASTVowel(ch) &&
           !isIASTCodaMark(ch) &&
           !isIASTCombiningMark(ch) &&
           !isIgnorableSymbol(ch);
}

void addSwaraIfPresent(Syllable& syllable, const Letter& letter) {
    if (letter.getSwaraType().has_value()) {
        syllable.addSwara(letter.getSwaraType().value());
    }
}

// Normalize tokens before parsing so trailing punctuation does not become a letter.
std::string cleanTokenForParsing(const std::string& token) {
    std::string cleaned = cleanWord(token);

    while (!cleaned.empty()) {
        auto chars = splitIAST(cleaned);
        if (chars.empty()) break;

        const std::string& tail = chars.back();
        if (!isIgnorableSymbol(tail)) break;

        cleaned.erase(cleaned.size() - tail.size());
    }

    return cleaned;
}

// When sandhi changes word boundaries, merge adjacent IAST words until the syllable counts line up.
Word mergeIASTWords(const std::vector<Word>& words, size_t start, size_t end) {
    std::string combined;
    for (size_t i = start; i < end; ++i) {
        combined += words[i].getText();
    }
    Word merged = buildWordIAST(combined);
    std::string combinedUnderlying;
    std::string combinedUnderlyingDev;
    for (size_t i = start; i < end; ++i) {
        if (i > start) {
            combinedUnderlying += " ";
            combinedUnderlyingDev += " ";
        }
        combinedUnderlying += words[i].getUnderlyingText();
        combinedUnderlyingDev += words[i].getUnderlyingDevText();
    }
    merged.setUnderlyingText(combinedUnderlying);
    merged.setUnderlyingDevText(combinedUnderlyingDev);
    return merged;
}

void finalizeVerseAlignment(Verse& verse, Hymn& hymn) {
    auto& devWords = verse.getDevWordsMutable();
    auto& iastWords = verse.getIASTWordsMutable();
    verse.clearSandhiBoundaries();

    if (devWords.size() != iastWords.size()) {
        const auto mismatchCount = static_cast<long long>(devWords.size()) - static_cast<long long>(iastWords.size());
        std::cout << mismatchCount << " Sandhi mismatches detected\n";
    }

    size_t devIndex = 0;
    size_t iastIndex = 0;

    while (devIndex < devWords.size() && iastIndex < iastWords.size()) {
        const size_t start = iastIndex;
        const size_t devSyllableCount = devWords[devIndex].getSyllables().size();

        // Grow the IAST span until it can be aligned against the current DEV word.
        Word mergedIAST = mergeIASTWords(iastWords, start, iastIndex + 1);
        ++iastIndex;

        while (iastIndex < iastWords.size() &&
               mergedIAST.getIASTSyllables().size() < devSyllableCount) {
            mergedIAST = mergeIASTWords(iastWords, start, iastIndex + 1);
            ++iastIndex;
        }

        alignWord(devWords[devIndex], mergedIAST);
        devWords[devIndex].setAlignedIAST(mergedIAST.getText());
        devWords[devIndex].setAlignedIASTUnderlying(mergedIAST.getUnderlyingText());
        devWords[devIndex].setUnderlyingDevText(mergedIAST.getUnderlyingDevText());

        if (iastIndex - start > 1) {
            for (size_t boundaryIndex = start; boundaryIndex + 1 < iastIndex; ++boundaryIndex) {
                verse.addSandhiBoundary(buildSandhiBoundary(
                    devWords[devIndex],
                    iastWords,
                    devIndex,
                    boundaryIndex,
                    boundaryIndex + 1));
            }
        }
        ++devIndex;
    }

    hymn.addVerse(verse);
}
}

std::vector<Syllable> buildSyllables(const Word& word) {
    std::vector<Syllable> syllables;
    const auto& letters = word.getLetters();

    Syllable current;
    bool hasNucleus = false;

    for (size_t i = 0; i < letters.size(); ++i) {
        const Letter& l = letters[i];
        std::string val = l.getValue();

        if (isIgnorableSymbol(val)) {
            continue;
        }

        // Case 1: independent vowel
        if (isVowel(val)) {
            if (hasNucleus) {
                syllables.push_back(current);
                current = Syllable();
            }

            current.setNucleus(l);
            hasNucleus = true;

            if (l.getSwaraType().has_value()) {
                current.addSwara(l.getSwaraType().value());
            }

            if (isLongVowel(val)) {
                current.setWeight("heavy");
            } else {
                current.setWeight("light");
            }

            continue;
        }

        // Case 2: vowel sign belongs to previous consonant nucleus
        if (isDevDependentVowel(val)) {
            if (!hasNucleus) {
                current.setNucleus(l);
                hasNucleus = true;
            }

            if (isDevLongDependentVowel(val)) {
                current.setWeight("heavy");
            } else {
                current.setWeight("light");
            }

            continue;
        }

        // Case 3: coda marks close the current syllable and make it heavy.
        if (isDevCodaMark(val)) {
            if (hasNucleus) {
                current.addCoda(l);
                current.setWeight("heavy");
            }
            continue;
        }

        // Case 4: virama suppresses inherent vowel
        if (isVirama(val)) {
            continue;
        }

        // Case 5: consonant
        if (!isDevConsonant(val)) {
            continue;
        }

        bool nextIsVirama = (i + 1 < letters.size() && isVirama(letters[i + 1].getValue()));
        bool nextIsVowelSign = (i + 1 < letters.size() && isDevDependentVowel(letters[i + 1].getValue()));

        // standalone consonant before nucleus
        if (!hasNucleus) {
            // consonant with explicit vowel sign or inherent vowel starts a syllable
            if (nextIsVowelSign || !nextIsVirama) {
                current.addOnset(l);

                // if no virama and no prior nucleus, this consonant carries inherent 'a'
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
            // after nucleus, consonants tend to go to coda until next syllable
            current.addCoda(l);

            if (!current.getCoda().empty()) {
                current.setWeight("heavy");
            }

            // if this consonant is not virama-joined, next syllable may start soon
            if (!nextIsVirama && !nextIsVowelSign) {
                syllables.push_back(current);
                current = Syllable();
                hasNucleus = false;
            }
        }
    }

    if (hasNucleus) {
        syllables.push_back(current);
    }

    return syllables;
}
//
std::vector<Syllable> buildSyllablesPada(const Word& word) {
    return buildSyllables(word);
}

Word buildWordDEV(const std::string& raw) {
    Word w(raw);
    w.setUnderlyingDevText(raw);
    auto chars = splitUTF8(raw);

    Letter currentLetter("");
    bool hasPending = false;

    for (const auto& ch : chars) {
        // Accent marks modify the previously seen letter instead of starting a new one.
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

std::vector<Syllable> buildSyllablesIAST(const Word& word) {
    std::vector<Syllable> syllables;
    const auto& letters = word.getLetters();
    std::vector<Letter> pendingOnset;

    for (size_t i = 0; i < letters.size(); ++i) {
        const Letter& letter = letters[i];
        const std::string value = letter.getValue();

        if (isIgnorableSymbol(value) || isIASTCombiningMark(value)) {
            continue;
        }

        if (isIASTCodaMark(value)) {
            if (!syllables.empty()) {
                syllables.back().addCoda(letter);
                syllables.back().setWeight("heavy");
            }
            continue;
        }

        if (isIASTConsonant(value)) {
            pendingOnset.push_back(letter);
            continue;
        }

        if (isIASTVowel(value)) {
            Syllable syllable;
            for (const auto& onset : pendingOnset) {
                syllable.addOnset(onset);
            }
            pendingOnset.clear();

            syllable.setNucleus(letter);
            syllable.setWeight(isIASTLongVowelValue(value) ? "heavy" : "light");

            while (i + 1 < letters.size() && isIASTCodaMark(letters[i + 1].getValue())) {
                ++i;
                syllable.addCoda(letters[i]);
                syllable.setWeight("heavy");
            }

            syllables.push_back(syllable);
        }
    }

    if (!pendingOnset.empty() && !syllables.empty()) {
        for (const auto& trailing : pendingOnset) {
            syllables.back().addCoda(trailing);
        }
        syllables.back().setWeight("heavy");
    }

    return syllables;
}

Word buildWordIAST(const std::string& raw) {
    Word w(raw);
    w.setUnderlyingText(raw);
    w.setUnderlyingDevText(transliterateIASTToDEV(raw));
    auto chars = splitIAST(raw);

    for (const auto& ch : chars) {
        if (isIASTCombiningMark(ch)) {
            continue;
        }

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

void alignWord(Word& devWord, Word& iastWord) {
    devWord.clearAlignment();

    const auto& devSylls = devWord.getSyllables();
    const auto& iastSylls = iastWord.getIASTSyllables();
    const size_t n = std::min(devSylls.size(), iastSylls.size());

    // Preserve only the aligned syllable pairs that exist on both sides.
    for (size_t i = 0; i < n; ++i) {
        SyllableAlignment a;
        a.dev = devSylls[i];
        a.iast = iastSylls[i];
        devWord.addAlignment(a);
    }
}

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
    // Parse the hymn file as a simple tagged text format.
    for (const auto& line : lines) {
        if (std::regex_search(line, match, mandala_re)) {
            hymn.setMandala(std::stoi(match[1]));
        } else if (std::regex_search(line, match, sukta_re)) {
            hymn.setSukta(std::stoi(match[1]));
        } else if (std::regex_search(line, match, rishi_re)) {
            hymn.addRishi(match[1]);
        } else if (std::regex_search(line, match, devata_re)) {
            hymn.addDevata(match[1]);
        } else if (std::regex_search(line, match, category_re)) {
            hymn.addCategory(match[1]);
        } else if (std::regex_search(line, match, verse_re)) {
            if (currentVerse.getVerseNumber() != 0) {
                finalizeVerseAlignment(currentVerse, hymn);
                currentVerse = Verse();
            }

            currentVerse.setVerseNumber(std::stoi(match[1]));
        } else if (line.rfind("DEV:", 0) == 0) {
            std::string devLine = line.substr(4);
            currentVerse.setDev(devLine);

            auto devWords = splitWords(devLine);
            for (const auto& w : devWords) {
                auto cleaned = cleanTokenForParsing(w);
                if (cleaned.empty()) continue;
                currentVerse.addDevWord(buildWordDEV(cleaned));
            }
        } else if (line.rfind("IAST:", 0) == 0) {
            std::string iastLine = line.substr(5);
            currentVerse.setIAST(iastLine);

            auto iastWords = splitWords(iastLine);
            for (const auto& w : iastWords) {
                auto cleaned = cleanTokenForParsing(w);
                if (cleaned.empty()) continue;
                currentVerse.addIASTWord(buildWordIAST(cleaned));
            }
        } else if (line.rfind("ENG:", 0) == 0) {
            currentVerse.setENG(line.substr(4));
        }
    }

    if (currentVerse.getVerseNumber() != 0) {
        finalizeVerseAlignment(currentVerse, hymn);
    }

    return hymn;
}
