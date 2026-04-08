//Poem Structures

#ifndef PSTRUCTS_H
#define PSTRUCTS_H

#include <string>
#include <vector>
#include <map>
#include <optional>
//#include "json.hpp"
//using json = nlohmann::json;

//Letter
class Letter {
private:
    std::string value;

    bool hasSwara; //SwaraCheck
    std::optional<std::string> swaraType; //Vector for swaraType (udatta anudatta svarita)

public:
    Letter(const std::string& v) : value(v), hasSwara(false), swaraType(std::nullopt) {}

    void setSwara(bool has, const std::optional<std::string>& type) {
        hasSwara = has;
        swaraType = type; }
    std::string getValue() const { return value; }

    bool getHasSwara() const { return hasSwara; }
    std::optional<std::string> getSwaraType() const { return swaraType; }
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

//Word

class Word {
private:
    std::string text;
    std::vector<Syllable> syllables;//Syllables in Word
    std::vector<Syllable> iastSyllables; //IAST Syllables only
    std::vector<Letter> letters; //Letters in Word
    std::vector<SyllableAlignment> alignment;//Align Syllables per Word
    std::string alignedIAST;


public:
    Word(const std::string& t) : text(t) {}

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



void setAlignedIAST(const std::string& s) {
    alignedIAST = s;
}

std::string getAlignedIAST() const {
    return alignedIAST;
}

};

//Verse

class Verse {
private:
    int verseNumber;
    std::string dev, iast, eng;

    //Store devWords and iastWords for split pipeline
    std::vector<Word> devWords;
    std::vector<Word> iastWords;

public:
    Verse() : verseNumber(0){}

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

    std::vector<Word>& getDevWordsMutable() {
    return devWords;
}

std::vector<Word>& getIASTWordsMutable() {
    return iastWords;
}

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
};




#endif
