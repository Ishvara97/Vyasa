//Poem Structures

#ifndef PSTRUCTS_H
#define PSTRUCTS_H

#include <string>
#include <vector>
//#include <sstream>
//#include <regex>

//Letter
class Letter {
private:
	std::string value;

public:
	Letter() = default;
	Letter(const std::string& val) : value(val) {}

	std::string getValue() const { return value; }
	void setValue(const std::string& val) { value = val; }
};

//Word
class Word {
private:
    std::string raw;
    std::vector<Letter> letters;

public:
    Word() = default;
    Word(const std::string& r) : raw(r) {}

    void addLetter(const Letter& l) {
        letters.push_back(l);
    }

    std::string getRaw() const { return raw; }
    void setRaw(const std::string& r) { raw = r; }

    const std::vector<Letter>& getLetters() const {
        return letters;
    }
};


//Verses
class Verse {
private:
	int number; //Verse Number
	std::string dev; //Devanagari
	std::string iast; //IAST
	std::string eng; //English

	std::vector<Word> words;

public:
	Verse() : number(0) {}

	void setNumber(int n) { number = n; }
	int getNumber() const { return number; }

    void setDev(const std::string& d) { dev = d; }
    std::string getDev() const { return dev; }

    void setIAST(const std::string& i) { iast = i; }
    std::string getIAST() const { return iast; }

    void setEng(const std::string& e) { eng = e; }
    std::string getEng() const { return eng; }

    void addWord(const Word& w) {
    	words.push_back(w);
    }

    const std::vector<Word>& getWords() const {
    	return words;
    }
};

//Hymn - Summation of Verses
class Hymn {
private:
	std::vector<Verse> verses;

public:
	void addVerse(const Verse& v) {
		verses.push_back(v);
	}

	const std::vector<Verse>& getVerses() const {
		return verses;
	}
};

#endif