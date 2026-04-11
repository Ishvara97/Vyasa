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

//Reverse
Letter jsonToLetter(const json& j) {
    Letter l(j["value"]);

    if (!j["swaraType"].is_null()) {
        l.setSwara(true, j["swaraType"]);
    }

    return l;
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


//Reverse
Syllable jsonToSyllable(const json& j) {
    Syllable s;

    for (const auto& l : j["onset"])
        s.addOnset(jsonToLetter(l));

    s.setNucleus(jsonToLetter(j["nucleus"]));

    for (const auto& l : j["coda"])
        s.addCoda(jsonToLetter(l));

    s.setWeight(j["weight"]);

    for (const auto& sw : j["swaras"])
        s.addSwara(sw);

    return s;
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

//Reverse
Word jsonToWord(const json& j) {
    return Word(j["text"]);
}
//Verse
json verseToJson(const Verse& v) {
    json j;

    j["verseNumber"] = v.getVerseNumber();
    j["dev"] = v.getDev();
    j["iast"] = v.getIAST();
    j["eng"] = v.getENG();

    // Words
    for (const auto& w : v.getDevWords())
        j["devWords"].push_back(wordToJson(w));

    for (const auto& w : v.getIASTWords())
        j["iastWords"].push_back(wordToJson(w));

    auto lf = getLetterFrequency(v);
    auto sf = getSwaraFrequency(v);

    json lf_json = json::object();
    for (const auto& [key, value] : lf) {
        lf_json[key] = value;
    }

    json sf_json = json::object();
    for (const auto& [key, value] : sf) {
        sf_json[key] = value;
    }

    j["analysis"]["letterFrequency"] = lf_json;
    j["analysis"]["swaraFrequency"] = sf_json;

    return j;
}

//Reverse
Verse jsonToVerse(const json& j) {
    Verse v;

    v.setVerseNumber(j["verseNumber"]);
    v.setDev(j["dev"]);
    v.setIAST(j["iast"]);
    v.setENG(j["eng"]);

   
    return v;

}

//Hymn
json hymnToJson(const Hymn& h) {
    json j;
    auto letterFreq = getHymnLetterFrequency(h);
    auto swaraFreq = getHymnSwaraFrequency(h);

    j["mandala"] = h.getMandala();
    j["sukta"] = h.getSukta();
    j["rishis"] = h.getRishis();
    j["devatas"] = h.getDevatas();
    j["categories"] = h.getCategories();

    for (const auto& v : h.getVerses())
        j["verses"].push_back(verseToJson(v));

    j["analysis"]["letterFrequency"] = letterFreq;
    j["analysis"]["swaraFrequency"] = swaraFreq;

    return j;
}

//Reverse
Hymn jsonToHymn(const json& j) {
    Hymn h;

    h.setMandala(j["mandala"]);
    h.setSukta(j["sukta"]);

    for (const auto& r : j["rishis"])
        h.addRishi(r);

    for (const auto& d : j["devatas"])
        h.addDevata(d);

    for (const auto& c : j["categories"])
        h.addCategory(c);

    for (const auto& vj : j["verses"])
        h.addVerse(jsonToVerse(vj));

    return h;
}