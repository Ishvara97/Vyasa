#include <iostream>
#include <fstream>
#include <sstream>

#include "Parser.h"
#include <windows.h> //To render Devanagari and IAST properly
#include "json.hpp"
using json = nlohmann::json;

int main() {
	SetConsoleOutputCP(CP_UTF8);
    std::ifstream file("Hymns/[Hymn 10.125 Svaras].txt");

    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    Hymn hymn = parseHymn(buffer.str());

    for (const auto& verse : hymn.getVerses()) {
        std::cout << "Verse " << verse.getNumber() << "\n";
        std::cout << "DEV: " << verse.getDev() << "\n";
        std::cout << "IAST: " << verse.getIAST() << "\n";
        std::cout << "ENG:  " << verse.getEng() << "\n";
        std::cout << "Words: " << "\n";
        for (const auto& word : verse.getIASTWords()) {
            for (const auto& letter : word.getLetters()) {
                std::cout << letter.getValue() << " ";
            }

            std::cout << "\n";
        }

        std::cout << "\nDevanagari Letter Frequency:\n";
        auto devFreq = verse.getDevLetterFrequency();

        for (const auto& pair : devFreq) {
        	std::cout << pair.first << " : " << pair.second << "\n";
        }
        std::cout << "\n";

        std::cout << "IAST Letter Frequency:\n";
        auto iastFreq = verse.getIASTLetterFrequency();

        for (const auto& pair : iastFreq) {
            std::cout << pair.first << " : " << pair.second << "\n";
        }
        std::cout << "\n";
    }
        std::cout << "\nHymn Total Devanagari Letter Frequency\n";

        auto hymnDevFreq = hymn.getDevLetterFrequency();

        for (const auto& pair : hymnDevFreq) {
            std::cout << pair.first << " : " << pair.second << "\n";
        }

        std::cout << "\nHymn Total IAST Letter Frequency\n";

        auto hymnIastFreq = hymn.getIASTLetterFrequency();

        for (const auto& pair : hymnIastFreq) {
            std::cout << pair.first << " : " << pair.second << "\n";
        }

        std::cout << "\nHymn Total Letter Frequency\n";

        auto hymnFreq = hymn.getLetterFrequency();

        for (const auto& pair : hymnFreq) {
            std::cout << pair.first << " : " << pair.second << "\n";
        }

        std::cout << "------------------\n";

        //IAST Letter Frequency

        //IAST Total Letter Frequency
    //JSON Import
        //std::ifstream in("hymn.json");
        //json j;
        //in >> j;

        //Hymn hymn = Hymn::fromJson(j);
    //JSON Export
        std::ofstream out("hymn.json");
        out << hymn.toJson().dump(4);
        out.close();
    return 0;

}
