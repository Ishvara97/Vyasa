//Poem Structures

#ifndef PSTRUCTS_H
#define PSTRUCTS_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include "json.hpp"
#include "CleanUp.h"
using json = nlohmann::json;

//LettersOnlyFilter
bool isAnalyzableLetter(const std::string& ch);

//Letter
class Letter {
private:
	std::string value; //Character
	bool hasSwara;
	std::optional<std::string> swaraType;

public:
	Letter(): hasSwara(false) {} //Disable if needed
	Letter(const std::string& val) : value(val), hasSwara(false) {} 
	

	
	void setValue(const std::string& val) { value = val; } //Writes Private Value
	std::string getValue() const { return value; } //Reads Private Value
	//Swara Confirmation
	void setHasSwara(bool val) { hasSwara = val; }
	bool getHasSwara() const { return hasSwara; }
	//Set Swara Value for Letter
	void setSwaraType(const std::optional<std::string>& type) {
        swaraType = type;
    }

    std::optional<std::string> getSwaraType() const {
        return swaraType;
    }

	//JSON Implementation
	json toJson() const {
        json j;
        j["value"] = value;
        j["hasSwara"] = hasSwara;

        if (swaraType.has_value())
            j["swaraType"] = swaraType.value();
        else
            j["swaraType"] = nullptr;

        return j;
    }
	
	static Letter fromJson(const json& j) {
		Letter l(j.at("value"));

		l.setHasSwara(j.at("hasSwara"));

		if (!j.at("swaraType").is_null()) {
			l.setSwaraType(j.at("swaraType").get<std::string>());
		}

		return l;
	}
};

//Word
class Word {
private:
    std::string raw; //Raw Input
    std::vector<Letter> letters; //Letters in Word -> e.g. Agni [a, g, n, i]
    int swaraCount; //# of Swaras in Word

public:
    Word() : swaraCount(0) {}
    Word(const std::string& r) : raw(r), swaraCount(0) {} //Paramterized Constructor for raw input

    void addLetter(const Letter& l) {
    	if (l.getHasSwara()) {			//Add to Swara Count for Word if Letter has Swara
    		swaraCount++;
    	}
    	if (l.getValue() == "|"||l.getValue() =="।" || l.getValue() == "॥") { //Removes Dandas showing up as Words
    		return;}   
    	else letters.push_back(l);
    }

    std::string getRaw() const { return raw; }
    void setRaw(const std::string& r) { raw = r; }
	const std::vector<Letter>& getLetters() const {
        return letters;
    }

    int getSwaraCount() const { return swaraCount; }

    //JSON Implementation
	json toJson() const {
     	json j;
        j["raw"] = raw;
        j["swaraCount"] = swaraCount;

        for (const auto& l : letters) {
            j["letters"].push_back(l.toJson());
        }

        return j;
    }

    static Word fromJson(const json& j) {
        Word w(j.at("raw"));

        for (const auto& l : j.at("letters")) {
            w.addLetter(Letter::fromJson(l));
        }

        return w;
    }

};


//Verses
class Verse {
private:
	int number; //Verse Number
	std::string dev, iast, eng; //Devanagari IAST English
	std::vector<Word> words; //Words in Verse
	std::string meter; //Verse Meter
	std::vector<Word> devWords; //Words in Devanagari
	std::vector<Word> iastWords;//Words in IAST

public:
	Verse() : number(0) {}

	void setNumber(int n) { number = n; }
	int getNumber() const { return number; }

    void setDev(const std::string& d) { dev = d; }
    void setIAST(const std::string& i) { iast = i; }
    void setEng(const std::string& e) { eng = e; }
    void setMeter(const std::string& m) { meter = m; }
    
    

	const std::vector<Word>& getDevWords() const;
	const std::vector<Word>& getIASTWords() const;
    
    std::string getDev() const { return dev; }
    std::string getIAST() const { return iast; }
    std::string getEng() const { return eng; }
    std::string getMeter() const {return meter; }
    //Frequency
    std::map<std::string, int> getDevLetterFrequency() const;
	std::map<std::string, int> getIASTLetterFrequency() const;
    void addWord(const Word& w) {
    	words.push_back(w);
    }

    const std::vector<Word>& getWords() const {
    	return words;
    }

    void addDevWord(const Word& w) { devWords.push_back(w);}
	void addIASTWord(const Word& w) {iastWords.push_back(w);}
    //IAST Frequenecy
    //auto iastWords = splitWords(v.getIAST());

	//for (auto& wstr : iastWords) {
	//    v.addIASTWord(buildWordIAST(wstr));
	//}

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

    //JSON Implementation
    json toJson() const {
        json j;
        j["number"] = number;
        j["dev"] = dev;
        j["iast"] = iast;
        j["eng"] = eng;
        j["meter"] = meter;

        for (const auto& w : words) {
            j["words"].push_back(w.toJson());
        }

        return j;
    }

    static Verse fromJson(const json& j) {
        Verse v;
        v.setNumber(j.at("number"));
        v.setDev(j.at("dev"));
        v.setIAST(j.at("iast"));
        v.setEng(j.at("eng"));
        v.setMeter(j.value("meter", ""));

        for (const auto& w : j.at("words")) {
            v.addWord(Word::fromJson(w));
        }

        return v;
    }
};

//Hymn - Summation of Verses
class Hymn {
private:
	std::string deity; //Deva Hymn is addressed to, change structure later from string to accomodate multiple devatas for filtering by devata [Agni, Indra]
	std::string rishi; //Rishi attributed to Hymn's composition to.
	std::optional<std::string> theme; //If theme of Hymn is applicable (e.g. Praise, Invocation, Speculation, etc.)
	int mandala;
	int sukta;

	std::vector<Verse> verses;

public:

	Hymn() : mandala(0), sukta(0) {}

	void setMandala(int m) { mandala = m; }
	void setSukta(int s) { sukta = s; }

	void setDeity(const std::string& d) { deity = d; }
    void setRishi(const std::string& r) { rishi = r; }
    void setTheme(const std::optional<std::string>& t) { theme = t; }

    std::string getDeity() const { return deity; }
    std::string getRishi() const { return rishi; }
    std::optional<std::string> getTheme() const { return theme; }

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

    //JSON Implementation
    json toJson() const {
        json j;
        j["mandala"] = mandala;
        j["sukta"] = sukta;
        j["deity"] = deity;
        j["rishi"] = rishi;

        if (theme.has_value()){ //If Theme attributed then include in JSON, NULL for no theme.
        	j["theme"] = theme.value();
        }
        else {
        	j["theme"] = nullptr;
        }

        for (const auto& v : verses) {
            j["verses"].push_back(v.toJson());
        }

        return j;
    }

    static Hymn fromJson(const json& j) {
        Hymn h;
        h.setMandala(j.value("mandala", 0)); //Set Mandala #
        h.setSukta(j.value("sukta", 0)); //Set Sukta #
        h.setDeity(j.at("deity")); //Set Deity
        h.setRishi(j.at("rishi")); // Set Rishi

        if (!j.at("theme").is_null()) {
            h.setTheme(j.at("theme").get<std::string>());
        }

        for (const auto& v : j.at("verses")) {
            h.addVerse(Verse::fromJson(v));
        }

        return h;
    }

};

#endif