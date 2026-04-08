#include "jsonserialization.h"

//Letter
json letterToJson(const Letter& l) {
    json j;
    j["value"] = l.getValue();
    j["hasSwara"] = l.getHasSwara();

    if (l.getSwaraType().has_value())
        j["swaraType"] = l.getSwaraType().value();
    else
        j["swaraType"] = nullptr;

    return j;
}

//Syllable
json syllableToJson(const Syllable& s) {
    json j;

    for (const auto& l : s.getOnset())
        j["onset"].push_back(letterToJson(l));

    j["nucleus"] = letterToJson(s.getNucleus());

    for (const auto& l : s.getCoda())
        j["coda"].push_back(letterToJson(l));

    j["weight"] = s.getWeight();
    j["swaras"] = s.getSwaras();

    return j;
}

//Word
json wordToJson(const Word& w) {
    json j;

    j["text"] = w.getText();

    for (const auto& l : w.getLetters())
        j["letters"].push_back(letterToJson(l));

    for (const auto& s : w.getSyllables())
        j["devSyllables"].push_back(syllableToJson(s));

    for (const auto& s : w.getIASTSyllables())
        j["iastSyllables"].push_back(syllableToJson(s));

    return j;
}

//Verse
json verseToJson(const Verse& v) {
    json j;

    j["verseNumber"] = v.getVerseNumber();
    j["dev"] = v.getDev();
    j["iast"] = v.getIAST();
    j["eng"] = v.getENG();

    for (const auto& w : v.getDevWords())
        j["devWords"].push_back(wordToJson(w));

    for (const auto& w : v.getIASTWords())
        j["iastWords"].push_back(wordToJson(w));

    return j;
}

//Hymn
json hymnToJson(const Hymn& h) {
    json j;

    j["mandala"] = h.getMandala();
    j["sukta"] = h.getSukta();
    j["rishis"] = h.getRishis();
    j["devatas"] = h.getDevatas();
    j["categories"] = h.getCategories();

    for (const auto& v : h.getVerses())
        j["verses"].push_back(verseToJson(v));

    return j;
}