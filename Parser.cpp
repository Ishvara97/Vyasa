#include "Parser.h"
#include "CleanUp.h"

#include <sstream>
#include <regex>

Word buildWord(const std::string& raw) {
	Word w(raw);

    auto letters = splitUTF8(raw);

    for (auto& l : letters) {
        w.addLetter(Letter(l));
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

    auto words = splitWords(v.getIAST());

    for (auto& wstr : words) {
        v.addWord(buildWord(wstr));
    }

    return v;
}

Hymn parseHymn(const std::string& text) {
    Hymn hymn;

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

    return hymn;
}
