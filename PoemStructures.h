//Poem Structures

#ifndef PSTRUCTS_H
#define PSTRUCTS_H

#include <string>
#include <vector>
#include <map>

//#include "json.hpp"
//using json = nlohmann::json;

//Verse

class Verse {
private:
    int verseNumber;
    std::string dev, iast, eng;

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
