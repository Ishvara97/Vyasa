#include "jsonserialization.h"

namespace {
// Convert map-based analyses into stable JSON objects.
json mapToJson(const std::map<std::string, int>& values) {
    json result = json::object();
    for (const auto& [key, value] : values) {
        result[key] = value;
    }
    return result;
}
}

json letterToJson(const Letter& l) {
    json j;
    const PhonemeFeatures phoneme = l.getPhoneme();

    j["value"] = l.getValue();
    j["hasSwara"] = l.getHasSwara();
    j["swaraType"] = l.getSwaraType().has_value() ? json(l.getSwaraType().value()) : json(nullptr);
    j["phonemeClass"] = phonemeClassToString(phoneme);
    j["vowelLength"] = vowelLengthToString(phoneme.vowelLength);

    // Include both the simple string labels and the structured phoneme payload.
    j["phoneme"] = {
        {"type", phonemeTypeToString(phoneme.type)},
        {"class", phonemeClassToString(phoneme)},
        {"consonantClass", consonantClassToString(phoneme.consonantClass)},
        {"vowelLength", vowelLengthToString(phoneme.vowelLength)},
        {"base", phoneme.base}
    };

    return j;
}

Letter jsonToLetter(const json& j) {
    Letter l(j.at("value").get<std::string>());

    if (j.contains("swaraType") && !j["swaraType"].is_null()) {
        l.setSwara(true, j["swaraType"].get<std::string>());
    }

    return l;
}

json syllableToJson(const Syllable& s) {
    json j;
    j["onset"] = json::array();
    j["coda"] = json::array();

    for (const auto& l : s.getOnset()) {
        j["onset"].push_back(letterToJson(l));
    }

    j["nucleus"] = letterToJson(s.getNucleus());

    for (const auto& l : s.getCoda()) {
        j["coda"].push_back(letterToJson(l));
    }

    j["weight"] = s.getWeight();
    j["swaras"] = s.getSwaras();
    return j;
}

Syllable jsonToSyllable(const json& j) {
    Syllable s;

    for (const auto& l : j.at("onset")) {
        s.addOnset(jsonToLetter(l));
    }

    s.setNucleus(jsonToLetter(j.at("nucleus")));

    for (const auto& l : j.at("coda")) {
        s.addCoda(jsonToLetter(l));
    }

    s.setWeight(j.at("weight").get<std::string>());

    for (const auto& sw : j.at("swaras")) {
        s.addSwara(sw.get<std::string>());
    }

    return s;
}

json wordToJson(const Word& w) {
    json j;
    j["text"] = w.getText();
    j["alignedIAST"] = w.getAlignedIAST();
    j["letters"] = json::array();
    j["devSyllables"] = json::array();
    j["iastSyllables"] = json::array();
    j["alignment"] = json::array();

    for (const auto& l : w.getLetters()) {
        j["letters"].push_back(letterToJson(l));
    }

    for (const auto& s : w.getSyllables()) {
        j["devSyllables"].push_back(syllableToJson(s));
    }

    for (const auto& s : w.getIASTSyllables()) {
        j["iastSyllables"].push_back(syllableToJson(s));
    }

    // Preserve the DEV<->IAST syllable correspondence used by the CSV exports and console output.
    for (const auto& alignment : w.getAlignment()) {
        j["alignment"].push_back({
            {"dev", syllableToJson(alignment.dev)},
            {"iast", syllableToJson(alignment.iast)}
        });
    }

    return j;
}

Word jsonToWord(const json& j) {
    return Word(j.at("text").get<std::string>());
}

json verseToJson(const Verse& v) {
    json j;
    j["verseNumber"] = v.getVerseNumber();
    j["dev"] = v.getDev();
    j["iast"] = v.getIAST();
    j["eng"] = v.getENG();
    j["devWords"] = json::array();
    j["iastWords"] = json::array();

    for (const auto& w : v.getDevWords()) {
        j["devWords"].push_back(wordToJson(w));
    }

    for (const auto& w : v.getIASTWords()) {
        j["iastWords"].push_back(wordToJson(w));
    }

    // Bundle verse-level analysis alongside the raw text and tokenization results.
    j["analysis"] = {
        {"letterFrequency", mapToJson(getLetterFrequency(v))},
        {"swaraFrequency", mapToJson(getSwaraFrequency(v))},
        {"phonemeClassFrequency", mapToJson(getPhonemeClassFrequency(v))}
    };

    return j;
}

Verse jsonToVerse(const json& j) {
    Verse v;
    v.setVerseNumber(j.at("verseNumber").get<int>());
    v.setDev(j.at("dev").get<std::string>());
    v.setIAST(j.at("iast").get<std::string>());
    v.setENG(j.at("eng").get<std::string>());
    return v;
}

json hymnToJson(const Hymn& h) {
    json j;
    j["mandala"] = h.getMandala();
    j["sukta"] = h.getSukta();
    j["rishis"] = h.getRishis();
    j["devatas"] = h.getDevatas();
    j["categories"] = h.getCategories();
    j["verses"] = json::array();

    for (const auto& v : h.getVerses()) {
        j["verses"].push_back(verseToJson(v));
    }

    // Add hymn-level rollups so JSON consumers do not need to recompute them.
    j["analysis"] = {
        {"totalLetterFrequency", mapToJson(getHymnLetterFrequency(h))},
        {"totalSwaraFrequency", mapToJson(getHymnSwaraFrequency(h))},
        {"totalPhonemeClassFrequency", mapToJson(getHymnPhonemeClassFrequency(h))}
    };

    return j;
}

Hymn jsonToHymn(const json& j) {
    Hymn h;
    h.setMandala(j.at("mandala").get<int>());
    h.setSukta(j.at("sukta").get<int>());

    for (const auto& r : j.at("rishis")) {
        h.addRishi(r.get<std::string>());
    }

    for (const auto& d : j.at("devatas")) {
        h.addDevata(d.get<std::string>());
    }

    for (const auto& c : j.at("categories")) {
        h.addCategory(c.get<std::string>());
    }

    for (const auto& vj : j.at("verses")) {
        h.addVerse(jsonToVerse(vj));
    }

    return h;
}
