// Poem Structures

#ifndef PSTRUCTS_H
#define PSTRUCTS_H

#include <map>
#include <optional>
#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

enum class PhonemeType {
    Vowel,
    Consonant,
    Other
};

enum class VowelLength {
    Short,
    Long,
    Diphthong,
    None
};

enum class ConsonantClass {
    Velar,
    Palatal,
    Retroflex,
    Dental,
    Labial,
    Nasal,
    Semivowel,
    Liquid,
    Sibilant,
    Aspirate,
    None
};

struct PhonemeFeatures {
    PhonemeType type = PhonemeType::Other;
    VowelLength vowelLength = VowelLength::None;
    ConsonantClass consonantClass = ConsonantClass::None;
    std::string base;
};

inline std::string phonemeTypeToString(PhonemeType type) {
    switch (type) {
        case PhonemeType::Vowel: return "Vowel";
        case PhonemeType::Consonant: return "Consonant";
        default: return "Other";
    }
}

inline std::string vowelLengthToString(VowelLength length) {
    switch (length) {
        case VowelLength::Short: return "Short";
        case VowelLength::Long: return "Long";
        case VowelLength::Diphthong: return "Diphthong";
        default: return "None";
    }
}

inline std::string consonantClassToString(ConsonantClass consonantClass) {
    switch (consonantClass) {
        case ConsonantClass::Velar: return "Velar";
        case ConsonantClass::Palatal: return "Palatal";
        case ConsonantClass::Retroflex: return "Retroflex";
        case ConsonantClass::Dental: return "Dental";
        case ConsonantClass::Labial: return "Labial";
        case ConsonantClass::Nasal: return "Nasal";
        case ConsonantClass::Semivowel: return "Semivowel";
        case ConsonantClass::Liquid: return "Liquid";
        case ConsonantClass::Sibilant: return "Sibilant";
        case ConsonantClass::Aspirate: return "Aspirate";
        default: return "None";
    }
}

inline std::string phonemeClassToString(const PhonemeFeatures& phoneme) {
    if (phoneme.type == PhonemeType::Vowel) {
        return "Vowel";
    }

    if (phoneme.type == PhonemeType::Consonant) {
        return consonantClassToString(phoneme.consonantClass);
    }

    return "Other";
}

inline PhonemeFeatures classifyPhoneme(const std::string& ch) {
    PhonemeFeatures phoneme;
    phoneme.base = ch;

    auto setVowel = [&](VowelLength length, const std::string& base) {
        phoneme.type = PhonemeType::Vowel;
        phoneme.vowelLength = length;
        phoneme.consonantClass = ConsonantClass::None;
        phoneme.base = base;
    };

    auto setConsonant = [&](ConsonantClass consonantClass, const std::string& base) {
        phoneme.type = PhonemeType::Consonant;
        phoneme.vowelLength = VowelLength::None;
        phoneme.consonantClass = consonantClass;
        phoneme.base = base;
    };

    if (ch == u8"\u0905" || ch == "a") setVowel(VowelLength::Short, "a");
    else if (ch == u8"\u0906" || ch == u8"\u093e" || ch == u8"\u0101") setVowel(VowelLength::Long, "a");
    else if (ch == u8"\u0907" || ch == u8"\u093f" || ch == "i") setVowel(VowelLength::Short, "i");
    else if (ch == u8"\u0908" || ch == u8"\u0940" || ch == u8"\u012b") setVowel(VowelLength::Long, "i");
    else if (ch == u8"\u0909" || ch == u8"\u0941" || ch == "u") setVowel(VowelLength::Short, "u");
    else if (ch == u8"\u090a" || ch == u8"\u0942" || ch == u8"\u016b") setVowel(VowelLength::Long, "u");
    else if (ch == u8"\u090b" || ch == u8"\u0943" || ch == u8"\u1e5b") setVowel(VowelLength::Short, "r");
    else if (ch == u8"\u0960" || ch == u8"\u0944" || ch == u8"\u1e5d") setVowel(VowelLength::Long, "r");
    else if (ch == u8"\u090c" || ch == u8"\u0962" || ch == u8"\u1e37") setVowel(VowelLength::Short, "l");
    else if (ch == u8"\u0961" || ch == u8"\u0963" || ch == u8"\u1e39") setVowel(VowelLength::Long, "l");
    else if (ch == u8"\u090f" || ch == u8"\u0947" || ch == "e") setVowel(VowelLength::Long, "e");
    else if (ch == u8"\u0913" || ch == u8"\u094b" || ch == "o") setVowel(VowelLength::Long, "o");
    else if (ch == u8"\u0910" || ch == u8"\u0948" || ch == "ai") setVowel(VowelLength::Diphthong, "ai");
    else if (ch == u8"\u0914" || ch == u8"\u094c" || ch == "au") setVowel(VowelLength::Diphthong, "au");
    else if (ch == u8"\u0915" || ch == u8"\u0916" || ch == u8"\u0917" || ch == u8"\u0918" ||
             ch == u8"\u0919" || ch == "k" || ch == "g" || ch == u8"\u1e45") {
        setConsonant(ch == u8"\u0919" || ch == u8"\u1e45" ? ConsonantClass::Nasal : ConsonantClass::Velar, "k");
    } else if (ch == u8"\u091a" || ch == u8"\u091b" || ch == u8"\u091c" || ch == u8"\u091d" ||
               ch == u8"\u091e" || ch == "c" || ch == "j" || ch == u8"\u00f1") {
        setConsonant(ch == u8"\u091e" || ch == u8"\u00f1" ? ConsonantClass::Nasal : ConsonantClass::Palatal, "c");
    } else if (ch == u8"\u091f" || ch == u8"\u0920" || ch == u8"\u0921" || ch == u8"\u0922" ||
               ch == u8"\u0923" || ch == u8"\u1e6d" || ch == u8"\u1e0d" || ch == u8"\u1e47") {
        setConsonant(ch == u8"\u0923" || ch == u8"\u1e47" ? ConsonantClass::Nasal : ConsonantClass::Retroflex, "t");
    } else if (ch == u8"\u0924" || ch == u8"\u0925" || ch == u8"\u0926" || ch == u8"\u0927" ||
               ch == u8"\u0928" || ch == "t" || ch == "d" || ch == "n") {
        setConsonant(ch == u8"\u0928" || ch == "n" ? ConsonantClass::Nasal : ConsonantClass::Dental, "t");
    } else if (ch == u8"\u092a" || ch == u8"\u092b" || ch == u8"\u092c" || ch == u8"\u092d" ||
               ch == u8"\u092e" || ch == "p" || ch == "b" || ch == "m") {
        setConsonant(ch == u8"\u092e" || ch == "m" ? ConsonantClass::Nasal : ConsonantClass::Labial, "p");
    } else if (ch == u8"\u092f" || ch == u8"\u0935" || ch == "y" || ch == "v") {
        setConsonant(ConsonantClass::Semivowel, ch == u8"\u092f" || ch == "y" ? "y" : "v");
    } else if (ch == u8"\u0930" || ch == u8"\u0932" || ch == "r" || ch == "l") {
        setConsonant(ConsonantClass::Liquid, ch == u8"\u0930" || ch == "r" ? "r" : "l");
    } else if (ch == u8"\u0936" || ch == u8"\u0937" || ch == u8"\u0938" || ch == u8"\u015b" || ch == u8"\u1e63" || ch == "s") {
        setConsonant(ConsonantClass::Sibilant, "s");
    } else if (ch == u8"\u0939" || ch == "h" || ch == u8"\u0903" || ch == u8"\u1e25") {
        setConsonant(ConsonantClass::Aspirate, "h");
    } else if (ch == u8"\u0902" || ch == u8"\u0901" || ch == u8"\u1e43" || ch == u8"\u1e41") {
        setConsonant(ConsonantClass::Nasal, "m");
    }

    return phoneme;
}

class Letter {
private:
    std::string value;
    bool hasSwara;
    std::optional<std::string> swaraType;
    PhonemeFeatures phoneme;

public:
    Letter(const std::string& v)
        : value(v), hasSwara(false), swaraType(std::nullopt), phoneme(classifyPhoneme(v)) {}

    void setSwara(bool has, const std::optional<std::string>& type) {
        hasSwara = has;
        swaraType = type;
    }

    std::string getValue() const { return value; }
    bool getHasSwara() const { return hasSwara; }
    std::optional<std::string> getSwaraType() const { return swaraType; }
    void setPhoneme(const PhonemeFeatures& p) { phoneme = p; }
    PhonemeFeatures getPhoneme() const { return phoneme; }
};

//Syllable
class Syllable {
private:
    Letter nucleus; //Nucleus Letter in Word
    std::vector<Letter> onset;//Onset Letters in Word
    std::vector<Letter> coda;//Coda letters in Word

    std::vector<std::string> swaras;//Swaras
    std::string weight; // light/heavy

public:
    Syllable() : nucleus("") {}
    void addOnset(const Letter& l) { onset.push_back(l); }
    void setNucleus(const Letter& l) { nucleus = l; }
    void addCoda(const Letter& l) { coda.push_back(l); }

    void addSwara(const std::string& s) { swaras.push_back(s); }
    void setWeight(const std::string& w) { weight = w; }

    const std::vector<Letter>& getOnset() const { return onset; }
    const Letter& getNucleus() const { return nucleus; }
    const std::vector<Letter>& getCoda() const { return coda; }

    const std::vector<std::string>& getSwaras() const { return swaras; }
    std::string getWeight() const { return weight; }

};

//Line Up DEV and IAST Syllables
struct SyllableAlignment {
    Syllable dev;
    Syllable iast;
};

struct SandhiBoundary {
    int surfaceWordIndex = -1;
    int leftUnderlyingWordIndex = -1;
    int rightUnderlyingWordIndex = -1;
    std::string surfaceDev;
    std::string surfaceIAST;
    std::string segmentedDev;
    std::string segmentedIAST;
    std::string leftUnderlyingDev;
    std::string leftUnderlyingIAST;
    std::string rightUnderlyingDev;
    std::string rightUnderlyingIAST;
    std::string category;
    std::string subtype;
    std::string confidence;
    std::string notes;
    std::string normalizationStatus;
    bool detected = false;
};

//Word

class Word {
private:
    std::string text;
    std::vector<Syllable> syllables;//Syllables in Word
    std::vector<Syllable> iastSyllables; //IAST Syllables only
    std::vector<Letter> letters; //Letters in Word
    std::vector<SyllableAlignment> alignment;//Align Syllables per Word
    std::string alignedIAST;
    std::string underlyingText;
    std::string alignedIASTUnderlying;
    std::string underlyingDevText;


public:
    Word(const std::string& t) : text(t), underlyingText(t), underlyingDevText(t) {}

    std::string getText() const { return text; }

    void addSyllable(const Syllable& s) {
        syllables.push_back(s); }

    void addLetter(const Letter& l) {
        letters.push_back(l); }

    void addIASTSyllable(const Syllable& s) {
    iastSyllables.push_back(s); }

    void addAlignment(const SyllableAlignment& a) {
    alignment.push_back(a); }

    void clearAlignment() {
    alignment.clear(); }

    const std::vector<Syllable>& getSyllables() const {
    return syllables;   }

    const std::vector<Letter>& getLetters() const {
        return letters; }

    const std::vector<Syllable>& getIASTSyllables() const {
    return iastSyllables; }

    const std::vector<SyllableAlignment>& getAlignment() const {
    return alignment; }

    void setAlignedIAST(const std::string& s) { alignedIAST = s; }
    void setUnderlyingText(const std::string& s) { underlyingText = s; }
    void setAlignedIASTUnderlying(const std::string& s) { alignedIASTUnderlying = s; }
    void setUnderlyingDevText(const std::string& s) { underlyingDevText = s; }
    std::string getAlignedIAST() const { return alignedIAST;}
    std::string getUnderlyingText() const { return underlyingText; }
    std::string getAlignedIASTUnderlying() const { return alignedIASTUnderlying; }
    std::string getUnderlyingDevText() const { return underlyingDevText; }

};

//Verse

class Verse {
private:
    int verseNumber;
    std::string dev, iast, eng;

    //Store devWords and iastWords for split pipeline
    std::vector<Word> devWords;
    std::vector<Word> iastWords;
    std::string meter;
    std::vector<SandhiBoundary> sandhiBoundaries;

public:
    Verse() : verseNumber(0), meter("No meter detected") {}

    void setVerseNumber(int n) { verseNumber = n; }
    int getVerseNumber() const { return verseNumber; }

    
    void setDev(const std::string& d) { dev = d; }
    std::string getDev() const { return dev; }
    void setIAST(const std::string& i) { iast = i; }
    std::string getIAST() const { return iast; }
    void setENG(const std::string& e) { eng = e; }
    std::string getENG() const { return eng; }
    //Dev/IAST Pipeline construction
    void addDevWord(const Word& w) { devWords.push_back(w); }
    void addIASTWord(const Word& w) { iastWords.push_back(w); }

    const std::vector<Word>& getDevWords() const { return devWords; }
    const std::vector<Word>& getIASTWords() const { return iastWords; }

    void setMeter(const std::string& m) { meter = m; }
    std::string getMeter() const { return meter; }
    void addSandhiBoundary(const SandhiBoundary& boundary) { sandhiBoundaries.push_back(boundary); }
    void clearSandhiBoundaries() { sandhiBoundaries.clear(); }
    const std::vector<SandhiBoundary>& getSandhiBoundaries() const { return sandhiBoundaries; }

    std::vector<Word>& getDevWordsMutable() { return devWords; }

    std::vector<Word>& getIASTWordsMutable() { return iastWords; }

};

//Hymn
class Hymn {
private:
    int mandala, sukta;
    std::vector<std::string> rishis, devatas, categories;

    std::vector<Verse> verses;    

public:
    Hymn() : mandala(0), sukta(0) {}

    void setMandala(int m) { mandala = m; }
    int getMandala() const { return mandala; }
    void setSukta(int s) { sukta = s; }
    int getSukta() const { return sukta; }

    void addRishi(const std::string& r) { rishis.push_back(r); }
    const std::vector<std::string>& getRishis() const { return rishis; }
    void addDevata(const std::string& d) { devatas.push_back(d); }
    const std::vector<std::string>& getDevatas() const { return devatas; }
    void addCategory(const std::string& c) { categories.push_back(c); }
    const std::vector<std::string>& getCategories() const { return categories; }

    void addVerse(const Verse& v) { verses.push_back(v); }
    const std::vector<Verse>& getVerses() const { return verses; }
    std::vector<Verse>& getVersesMutable() { return verses; }

};




#endif
