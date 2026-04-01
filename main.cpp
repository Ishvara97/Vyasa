#include "Parser.h"

#include <iostream>
#include <fstream>
#include <sstream>

int main() {
	std::ifstream file("[Hymn 1.1].txt");
	std::stringstream buffer;
	buffer << file.rdbuf();

	Hymn hymn = parseHymn(buffer.str());

	for (const auto& verse : hymn.verses) {
		std::cout << "Verse " << verse.number << "\n";
		std::cout << "IAST: " << verse.iast << "\n";

		for (const auto& word : verse.words) {
            std::cout << "  Word: " << word.raw << " -> Letters: ";

            for (const auto& letter : word.letters) {
                std::cout << letter.value << " ";
            }

            std::cout << "\n";
        }

        std::cout << "------------------\n";
    }

    return 0;
}

