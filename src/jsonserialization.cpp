#include "jsonserialization.h"
#include <sstream>

namespace {
// Convert map-based analyses into stable JSON objects.
json mapToJson(const std::map<std::string, int>& values) {
    json result = json::object();
    for (const auto& [key, value] : values) {
        result[key] = value;
    }
    return result;
}

json mapToJson(const std::map<std::string, double>& values) {
    json result = json::object();
    for (const auto& [key, value] : values) {
        result[key] = value;
    }
    return result;
}

std::string dumpJsonValue(const json& value) {
    return value.dump(2);
}

json sandhiBoundaryToJson(const SandhiBoundary& boundary) {
    return {
        {"surfaceWordIndex", boundary.surfaceWordIndex},
        {"leftUnderlyingWordIndex", boundary.leftUnderlyingWordIndex},
        {"rightUnderlyingWordIndex", boundary.rightUnderlyingWordIndex},
        {"surfaceDev", boundary.surfaceDev},
        {"surfaceIAST", boundary.surfaceIAST},
        {"segmentedDev", boundary.segmentedDev},
        {"segmentedIAST", boundary.segmentedIAST},
        {"leftUnderlyingDev", boundary.leftUnderlyingDev},
        {"leftUnderlyingIAST", boundary.leftUnderlyingIAST},
        {"rightUnderlyingDev", boundary.rightUnderlyingDev},
        {"rightUnderlyingIAST", boundary.rightUnderlyingIAST},
        {"category", boundary.category},
        {"subtype", boundary.subtype},
        {"confidence", boundary.confidence},
        {"notes", boundary.notes},
        {"normalizationStatus", boundary.normalizationStatus},
        {"detected", boundary.detected}
    };
}

json similarityScoreToJson(const SimilarityScore& score) {
    return {
        {"dotProduct", score.dotProduct},
        {"magnitudeA", score.magnitudeA},
        {"magnitudeB", score.magnitudeB},
        {"cosineSimilarity", score.cosineSimilarity},
        {"confidence", score.confidence}
    };
}

json verseSimilarityToJson(const VerseSimilarityComparison& comparison) {
    return {
        {"leftVerseNumber", comparison.leftVerseNumber},
        {"rightVerseNumber", comparison.rightVerseNumber},
        {"phonemeClass", similarityScoreToJson(comparison.phonemeClass)},
        {"swara", similarityScoreToJson(comparison.swara)},
        {"meterPattern", similarityScoreToJson(comparison.meterPattern)},
        {"bigramJaccard", comparison.bigramJaccard},
        {"trigramJaccard", comparison.trigramJaccard},
        {"rareWeightedBigram", similarityScoreToJson(comparison.rareWeightedBigram)},
        {"rareWeightedTrigram", similarityScoreToJson(comparison.rareWeightedTrigram)},
        {"bigramPosition", similarityScoreToJson(comparison.bigramPosition)},
        {"trigramPosition", similarityScoreToJson(comparison.trigramPosition)},
        {"leftBigramEntropy", comparison.leftBigramEntropy},
        {"rightBigramEntropy", comparison.rightBigramEntropy},
        {"leftTrigramEntropy", comparison.leftTrigramEntropy},
        {"rightTrigramEntropy", comparison.rightTrigramEntropy}
    };
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
    j["underlyingText"] = w.getUnderlyingText();
    j["underlyingDevText"] = w.getUnderlyingDevText();
    j["alignedIAST"] = w.getAlignedIAST();
    j["alignedIASTUnderlying"] = w.getAlignedIASTUnderlying();
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
    j["meter"] = v.getMeter();
    j["dev"] = v.getDev();
    j["iast"] = v.getIAST();
    j["eng"] = v.getENG();
    j["devWords"] = json::array();
    j["iastWords"] = json::array();
    j["sandhi"] = json::array();

    for (const auto& w : v.getDevWords()) {
        j["devWords"].push_back(wordToJson(w));
    }

    for (const auto& w : v.getIASTWords()) {
        j["iastWords"].push_back(wordToJson(w));
    }

    for (const auto& boundary : v.getSandhiBoundaries()) {
        j["sandhi"].push_back(sandhiBoundaryToJson(boundary));
    }

    // Bundle verse-level analysis alongside the raw text and tokenization results.
    j["analysis"] = {
        {"letterFrequency", mapToJson(getLetterFrequency(v))},
        {"swaraFrequency", mapToJson(getSwaraFrequency(v))},
        {"phonemeClassFrequency", mapToJson(getPhonemeClassFrequency(v))},
        {"meterPattern", getSyllablePattern(v)},
        {"phoneticBigrams", mapToJson(getPhoneticNGrams(v, 2))},
        {"phoneticTrigrams", mapToJson(getPhoneticNGrams(v, 3))},
        {"bigramEntropy", getPhoneticNGramEntropy(v, 2)},
        {"trigramEntropy", getPhoneticNGramEntropy(v, 3)},
        {"bigramPositionProfile", mapToJson(getPhoneticNGramPositionProfile(v, 2))},
        {"trigramPositionProfile", mapToJson(getPhoneticNGramPositionProfile(v, 3))}
    };

    return j;
}

Verse jsonToVerse(const json& j) {
    Verse v;
    v.setVerseNumber(j.at("verseNumber").get<int>());
    if (j.contains("meter")) {
        v.setMeter(j.at("meter").get<std::string>());
    }
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

    j["verseSimilarities"] = json::array();
    for (const auto& comparison : getVerseSimilarityComparisons(h)) {
        j["verseSimilarities"].push_back(verseSimilarityToJson(comparison));
    }

    // Add hymn-level rollups so JSON consumers do not need to recompute them.
    j["analysis"] = {
        {"totalLetterFrequency", mapToJson(getHymnLetterFrequency(h))},
        {"totalSwaraFrequency", mapToJson(getHymnSwaraFrequency(h))},
        {"totalPhonemeClassFrequency", mapToJson(getHymnPhonemeClassFrequency(h))},
        {"phoneticBigrams", mapToJson(getHymnPhoneticNGrams(h, 2))},
        {"phoneticTrigrams", mapToJson(getHymnPhoneticNGrams(h, 3))},
        {"bigramEntropy", getHymnPhoneticNGramEntropy(h, 2)},
        {"trigramEntropy", getHymnPhoneticNGramEntropy(h, 3)},
        {"bigramPositionProfile", mapToJson(getHymnPhoneticNGramPositionProfile(h, 2))},
        {"trigramPositionProfile", mapToJson(getHymnPhoneticNGramPositionProfile(h, 3))}
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

std::string hymnToJsonString(const Hymn& h) {
    return hymnToJson(h).dump(2);
}
