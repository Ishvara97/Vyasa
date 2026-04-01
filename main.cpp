#include <iostream>
#include <fstream>
#include <sstream>
#include "Parser.h"
#include <windows.h> //To render Devanagari and IAST properly

int main() {
	SetConsoleOutputCP(CP_UTF8);
    std::ifstream file("Hymns/[Hymn 10.125].txt");

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
        for (const auto& word : verse.getWords()) {
           // std::cout << letter.getLetters(word.getRaw()) << " \n";

            for (const auto& letter : word.getLetters()) {
                std::cout << letter.getValue() << " ";
            }

            std::cout << "\n";
        }
        std::cout << "\nLetter Frequency:\n";
        auto freq = verse.getLetterFrequency();

        for (const auto& pair : freq) {
        	std::cout << pair.first << " : " << pair.second << "\n";
        }

    }

    std::cout << "\n=== Hymn Letter Frequency ===\n";

        auto hymnFreq = hymn.getLetterFrequency();

        for (const auto& pair : hymnFreq) {
            std::cout << pair.first << " : " << pair.second << "\n";
        }

        std::cout << "------------------\n";

    return 0;
}