#include "Parser.h"
#include "CleanUp.h"
#include <iostream>
#include <fstream>
#include <regex>
//SWARA DETECTORS
const std::string DEV_SVARITA    = u8"॑";
const std::string DEV_ANUDATTA   = u8"॒";

const std::string COMB_SVARITA   = u8"\u030D";
const std::string COMB_ANUDATTA  = u8"\u0331";

//Syllable Parser
std::vector<Syllable> buildSyllables(const Word& word) {
    std::vector<Syllable> syllables;

    const auto& letters = word.getLetters();

    Syllable current;
    bool hasNucleus = false;

    for (size_t i = 0; i < letters.size(); ++i) {
        const Letter& l = letters[i];
        std::string val = l.getValue();

        if (isVowel(val)) {
            //If already had nucleus, new syllable
            if (hasNucleus) {
                syllables.push_back(current);
                current = Syllable();
            }

            current.setNucleus(l);
            hasNucleus = true;
            //Collect Vowel Weight now that Nucleus obtained
            if (isLongVowel(val)) {
                current.setWeight("heavy");
            } else {
                current.setWeight("light");
            }
            //Collect swara
            if (l.getSwaraType().has_value()) {
                current.addSwara(l.getSwaraType().value());
            }
        }

        else { if (!hasNucleus) { //If no Nucleus present, add as Onset, if, add as Coda
                current.addOnset(l);
            } else {
                current.addCoda(l);
            }
        }
    }
    //If has Nucleus, pushback to syllables.
    if (hasNucleus) {
            //If current is not Coda then set to Heavy
        if (!current.getCoda().empty()) {
            current.setWeight("heavy");
        }

        syllables.push_back(current);
    }

    return syllables;
}




//Dev WordBuilder w/ Swaras
Word buildWordDEV(const std::string& raw) {
    Word w(raw);//Raw Word Input

    auto chars = splitUTF8(raw);//Split UTF8 Raw

    Letter currentLetter("");
    bool hasPending = false;

    for (const auto& ch : chars) {

        // Swara marks (attach to previous letter) see top of file
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

            // New base character, finalize previous
            if (hasPending) {
                if (!currentLetter.getHasSwara()) {//If no Svarita or Anudatta assign Udatta
                    currentLetter.setSwara(false, "udatta");
                }
         w.addLetter(currentLetter);//After Swara evaluation add letter to Word
       }

        // Start next letter
        currentLetter = Letter(ch);
        hasPending = true;
    }

    // Final letter
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

    for (const auto& l : letters) {
        std::string val = l.getValue();

        if (isIASTVowel(val)) {

         if (hasNucleus) {
                syllables.push_back(current);
                current = Syllable();
            }

            current.setNucleus(l);
            hasNucleus = true;
        }
        else {
            if (!hasNucleus)
                current.addOnset(l);
            else
                current.addCoda(l);
        }
    }

    if (hasNucleus)
        syllables.push_back(current);

    return syllables;
}

//WordBuilder IAST (No  SwaraChecks)
Word buildWordIAST(const std::string& raw) {
    Word w(raw);

    auto chars = splitUTF8(raw);

    for (const auto& ch : chars) {
        Letter l(ch);
        l.setSwara(false, std::nullopt);//Swara set to False for IAST every letter
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

    const auto& devSylls = devWord.getSyllables();
    const auto& iastSylls = iastWord.getIASTSyllables();

    size_t n = std::min(devSylls.size(), iastSylls.size());

    std::cout << "ALIGNING WORD...\n";

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

    // Read file
    std::ifstream file(filename);
    std::string text((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

    auto lines = splitLines(text);//Perform SplitLines on File (CleanUp)

    Verse currentVerse; //Create a Verse called currentVerse
    //Establish Regular Expression for Hymn Properties
    std::regex mandala_re("Mandala:\\s*(\\d+)");
    std::regex sukta_re("Sukta:\\s*(\\d+)");
    std::regex rishi_re("Rishis:\\s*(.*)");
    std::regex devata_re("Devatas:\\s*(.*)");
    std::regex category_re("Categories:\\s*(.*)");
    std::regex verse_re("\\[Verse\\s*(\\d+)\\]");


    //
    std::smatch match;
    //For every line in lines (CleanUp) match initial text (above) property to regex and place it in.
    for (const auto& line : lines) {

        if (std::regex_search(line, match, mandala_re)) { //e.g. if line starts with "Mandala:\\s*(\\d+)" then set that Mandala String ("1") as the Mandala Integer 1 for the Hymn
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

            // Save previous verse if exists
            if (currentVerse.getVerseNumber() != 0) {

            // Alignment
            auto& devWords = currentVerse.getDevWordsMutable();
            auto& iastWords = currentVerse.getIASTWordsMutable();

            size_t n = std::min(devWords.size(), iastWords.size());

            for (size_t i = 0; i < n; ++i) {
                alignWord(devWords[i], iastWords[i]);
                devWords[i].setAlignedIAST(iastWords[i].getText());
            }

            hymn.addVerse(currentVerse);
            currentVerse = Verse();
        }

            currentVerse.setVerseNumber(std::stoi(match[1]));//Convert String to Integer
        }
        else if (line.rfind("DEV:", 0) == 0) { //When DEV: found, set current VerseDev to it, substring starting with 4th character on line.
            std::string devLine = line.substr(4);
            currentVerse.setDev(devLine);

            // Split into Dev words
            auto devWords = splitWords(devLine);

            for (const auto& w : devWords) {
                auto cleaned = cleanWord(w);
                if (cleaned.empty()) continue;
                if (isIgnorableSymbol(w)) continue;
                currentVerse.addDevWord(buildWordDEV(w));
                }
        }
        else if (line.rfind("IAST:", 0) == 0) {
            std::string iastLine = line.substr(5);
            currentVerse.setIAST(iastLine);

            // Split into IAST words
            auto iastWords = splitWords(iastLine);

            for (const auto& w : iastWords) {
                auto cleaned = cleanWord(w);
                if (cleaned.empty()) continue;
                if (isIgnorableSymbol(w)) continue;
                currentVerse.addIASTWord(buildWordIAST(w));
            } //5th Character for IAST
        }
        else if (line.rfind("ENG:", 0) == 0) {
            currentVerse.setENG(line.substr(4)); //4th Character for ENG:
        }
    }

    // Add last verse
    if (currentVerse.getVerseNumber() != 0) {

    auto& devWords = currentVerse.getDevWordsMutable();
    auto& iastWords = currentVerse.getIASTWordsMutable();

    size_t n = std::min(devWords.size(), iastWords.size());

    for (size_t i = 0; i < n; ++i) {
        alignWord(devWords[i], iastWords[i]);
    }

    hymn.addVerse(currentVerse);
    }

    auto& devWords = currentVerse.getDevWordsMutable();
    auto& iastWords = currentVerse.getIASTWordsMutable();

    size_t n = std::min(devWords.size(), iastWords.size());

    for (size_t i = 0; i < n; ++i) {
    alignWord(devWords[i], iastWords[i]);
    }


    return hymn;
}

