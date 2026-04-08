#include <iostream>
#include <string>
#include "Parser.h"
#include <windows.h>
#include "json.hpp"
using json = nlohmann::json;

namespace {
std::string syllableToText(const Syllable& s) {
    std::string text;

    for (const auto& l : s.getOnset()) {
        text += l.getValue();
    }

    text += s.getNucleus().getValue();

    for (const auto& l : s.getCoda()) {
        text += l.getValue();
    }

    return text;
}
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Hymn hymn = parseHymn("Hymns/[Hymn 10.125 Svaras].txt");

    std::cout << "\nMandala: " << hymn.getMandala() << "\n";
    std::cout << "Sukta: " << hymn.getSukta() << "\n\n";

    for (const auto& rishi : hymn.getRishis()) {
        std::cout << "Rishis:" << rishi << ", ";
    }
    std::cout << "\n";

    for (const auto& devata : hymn.getDevatas()) {
        std::cout << "Devatas:" << devata << ", ";
    }
    std::cout << "\n";

    for (const auto& category : hymn.getCategories()) {
        std::cout << "Categories:" << category << ", ";
    }
    std::cout << "\n\n";

    for (const auto& verse : hymn.getVerses()) {
        std::cout << "[Verse " << verse.getVerseNumber() << "]\n";
        std::cout << "DEV: " << verse.getDev() << "\n";
        std::cout << "IAST: " << verse.getIAST() << "\n";
        std::cout << "ENG: " << verse.getENG() << "\n\n";

        std::cout << "DEV Words:\n";
        for (const auto& w : verse.getDevWords()) {
            std::cout << "[" << w.getText() << "] ";
        }
        std::cout << "\n\n";

        std::cout << "IAST Words:\n";
        for (const auto& w : verse.getIASTWords()) {
            std::cout << "[" << w.getText() << "] ";
        }
        std::cout << "\n\n";

        std::cout << "DEV Letters:\n";
        for (const auto& w : verse.getDevWords()) {
            std::cout << w.getText() << " -> ";
            for (const auto& l : w.getLetters()) {
                std::cout << l.getValue();
                if (l.getSwaraType().has_value()) {
                    std::cout << "(" << l.getSwaraType().value() << ") ";
                } else {
                    std::cout << "(none) ";
                }
            }
            std::cout << "\n";
        }

        std::cout << "\nSyllables:\n\n";
        for (const auto& w : verse.getDevWords()) {
            std::cout << w.getText() << " -> ";

            for (const auto& s : w.getSyllables()) {
                std::cout << "[";
                for (const auto& l : s.getOnset()) {
                    std::cout << l.getValue();
                }
                std::cout << "|";
                std::cout << s.getNucleus().getValue();
                std::cout << "|";
                for (const auto& l : s.getCoda()) {
                    std::cout << l.getValue();
                }
                std::cout << ":" << s.getWeight() << "] ";
            }

            std::cout << "\n";
        }

        std::cout << "\nAligned Syllables:\n";
        for (const auto& w : verse.getDevWords()) {
            std::cout << w.getText() << " -> " << w.getAlignedIAST() << "\n";
            std::cout << "Alignment size: " << w.getAlignment().size() << "\n";

            for (const auto& a : w.getAlignment()) {
                std::cout << "[DEV: " << syllableToText(a.dev) << " <-> IAST: " << syllableToText(a.iast) << "] ";
            }

            std::cout << "\n\n";
        }
    }

    return 0;
}
