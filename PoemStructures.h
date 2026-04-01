//Poem Structures

#ifndef PSTRUCTS_H
#define PSTRUCTS_H

#include <string>
#include <vector>
#include <map>

#include "json.hpp"
using json = nlohmann::json;

//Letter
class Letter {
private:
	std::string value; //Character

public:
	Letter() = default;
	Letter(const std::string& val) : value(val) {} //Parameterized Constructor for value

	std::string getValue() const { return value; } //Reads Private Value
	void setValue(const std::string& val) { value = val; } //Writes Private Value
};

//Word
class Word {
private:
    std::string raw; //Raw Input
    std::vector<Letter> letters; //Letters in Word -> e.g. Agni [a, g, n, i]

public:
    Word() = default; 
    Word(const std::string& r) : raw(r) {} //Paramterized Constructor for raw input

    void addLetter(const Letter& l) {
    	if (l.getValue() == "|"||l.getValue() =="।" || l.getValue() == "॥") {
    		return;}   
    	else letters.push_back(l);
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

    // Letter Frequency
    std::map<std::string, int> getLetterFrequency() const {
    	std::map<std::string, int> freq;

    	for (const auto& word : words) {
    		for (const auto& letter : word.getLetters()) {
    			
    			std::string val = letter.getValue();
				if (val == "|"||val =="।" || val == "॥") {
                continue;
            }
            	freq[val]++;
    		}
    	}
    	return freq;
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
	// Total Hymn Letter Frequency
	std::map<std::string, int> getLetterFrequency() const {
        std::map<std::string, int> totalFreq;

        for (const auto& verse : verses) {
            auto verseFreq = verse.getLetterFrequency();

            for (const auto& pair : verseFreq) {
                totalFreq[pair.first] += pair.second;
            }
        }

        return totalFreq;
    }

};

#endif