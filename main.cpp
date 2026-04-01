#include <iostream>
#include <fstream>
#include <sstream>
#include "Parser.h"
#include <windows.h>

int main() {
	SetConsoleOutputCP(CP_UTF8);
    std::ifstream file("[Hymn 1.1].txt");

    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    Hymn hymn = parseHymn(buffer.str());

    for (const auto& verse : hymn.getVerses()) {
        std::cout << "Verse " << verse.getNumber() << "\n";
        std::cout << "IAST: " << verse.getIAST() << "\n";
        std::cout << "DEV: " << verse.getDev() << "\n";
        std::cout << "ENG:  " << verse.getEng() << "\n";
        std::cout << "Words: " << "\n";
        for (const auto& word : verse.getWords()) {
            std::cout << word.getRaw() << " -> Letters: ";

            for (const auto& letter : word.getLetters()) {
                std::cout << letter.getValue() << " ";
            }

            std::cout << "\n";
        }

        std::cout << "------------------\n";
    }

    return 0;
}