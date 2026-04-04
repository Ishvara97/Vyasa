#include "Parser.h"
#include "PoemStructures.h"
#include "CleanUp.h"
#include <optional>
#include <string>
#include <iostream>
#include <sstream>
#include <regex>


//IAST Specific WordBuilder
Word buildWordIAST(const std::string& raw) {
    Word w(raw);

    auto chars = splitUTF8(raw);

    for (const auto& ch : chars) {
        if (isIgnorableSymbol(ch)) continue;

        Letter l(ch);
        l.setHasSwara(false);
        l.setSwaraType(std::nullopt);

        w.addLetter(l);
    }

    return w;
}
//DEV
Word buildWord(const std::string& raw) {
	Word w(raw);

    //Just kill me.
    const std::string DEV_SVARITA  = u8"॑";   // U+0951
    const std::string DEV_ANUDATTA = u8"॒";   // U+0952

    const std::string COMB_SVARITA  = u8"\u030D"; // CC 8D
    const std::string COMB_ANUDATTA = u8"\u0331"; // CC B1

    auto letters = splitUTF8(raw);

    Letter currentLetter;
    bool hasPendingLetter = false;

    for (size_t i = 0; i < letters.size(); ++i){ //Swara Cycling
        const std::string ch = letters[i];

        if (isIgnorableSymbol(ch)) continue;

        std::cout << "Char: " << ch << " | size: " << ch.size() << "\n";

        for (unsigned char c : ch) {
            printf("%02X ", c);
        }
        std::cout << "\n";

        
        if (ch == DEV_SVARITA || ch == COMB_SVARITA || ch == DEV_ANUDATTA || ch == COMB_ANUDATTA) {

            if (hasPendingLetter) {
                 currentLetter.setHasSwara(true);

            if (ch == DEV_SVARITA || ch == COMB_SVARITA) {
            currentLetter.setSwaraType("svarita");
          } else {
            currentLetter.setSwaraType("anudatta");
        }
    }
    continue;
}

        //Udatta (Unmarked) Accounting
        if (hasPendingLetter) {
            if (!currentLetter.getHasSwara()) {
                currentLetter.setSwaraType("udatta");
            }

            w.addLetter(currentLetter);
        }

        currentLetter = Letter(ch);
        currentLetter.setHasSwara(false);
        currentLetter.setSwaraType(std::nullopt);
        hasPendingLetter = true;
    }

        if (hasPendingLetter) {
            if (!currentLetter.getHasSwara()) {
                currentLetter.setSwaraType("udatta");
            }

            w.addLetter(currentLetter);
        }

    return w;
}

Verse parseVerseBlock(const std::string& block) {
    Verse v;

    std::regex verse_num_re("\\[Verse (\\d+)\\]");
    std::regex dev_re("DEV:\\s*(.*)");
    std::regex iast_re("IAST:\\s*(.*)");
    std::regex eng_re("ENG:\\s*(.*)");

    std::smatch match;

    if (std::regex_search(block, match, verse_num_re)) {
        v.setNumber(std::stoi(match[1]));
    }

    if (std::regex_search(block, match, dev_re)) {
        v.setDev(match[1]);
    }

    if (std::regex_search(block, match, iast_re)) {
        v.setIAST(match[1]);
    }

    if (std::regex_search(block, match, eng_re)) {
        v.setEng(match[1]);
    }

    // DEV parsing
auto devWords = splitWords(v.getDev());

for (auto& wstr : devWords) {
    v.addDevWord(buildWord(wstr));
}

// IAST parsing
auto iastWords = splitWords(v.getIAST());

for (auto& wstr : iastWords) {
    v.addIASTWord(buildWordIAST(wstr));
}

    return v;
}

Hymn parseHymn(const std::string& text) {
    Hymn hymn;
    std::regex mandala_re("Mandala:\\s*(\\d+)");
    std::regex sukta_re("Sukta:\\s*(\\d+)");
    std::regex deity_re("Deity:\\s*(.*)");
    std::regex rishi_re("Rishi:\\s*(.*)");
    std::regex theme_re("Theme:\\s*(.*)");
    std::regex verse_split("\\[Verse \\d+\\]");
    std::sregex_iterator it(text.begin(), text.end(), verse_split);
    std::sregex_iterator end;

    std::vector<size_t> positions;

    for (; it != end; ++it) {
        positions.push_back(it->position());
    }

    for (size_t i = 0; i < positions.size(); ++i) {
        size_t start = positions[i];
        size_t end_pos = (i + 1 < positions.size()) ? positions[i + 1] : text.size();

        std::string block = text.substr(start, end_pos - start);
        hymn.addVerse(parseVerseBlock(block));
    }

    std::smatch match;

    if (std::regex_search(text, match, mandala_re)) {
        hymn.setMandala(std::stoi(match[1]));
    }

    if (std::regex_search(text, match, sukta_re)) {
        hymn.setSukta(std::stoi(match[1]));
    }

    if (std::regex_search(text, match, deity_re)) {
        hymn.setDeity(match[1]);
    }

    if (std::regex_search(text, match, rishi_re)) {
        hymn.setRishi(match[1]);
    }

    if (std::regex_search(text, match, theme_re)) {
        std::string t = match[1];
        if (!t.empty()) {
            hymn.setTheme(t);
        }
    }

    return hymn;
}
