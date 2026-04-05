#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "Parser.h"
#include <windows.h> //To render Devanagari and IAST properly
#include "json.hpp"
using json = nlohmann::json;

int main() {
    //Ensure Devanagari Inputs/Outputs Properly
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

	Hymn hymn = parseHymn("Hymns/[Hymn 10.125 Svaras].txt"); //Parse Hymn on File

    std::cout << "\nMandala: " << hymn.getMandala() << "\n"; //Output Mandala#
    std::cout << "Sukta: " << hymn.getSukta() << "\n\n"; //Output Sukta#
    //Output Metadata from Vectors
    for (const auto& rishi : hymn.getRishis()){
        std::cout <<"Rishis:" << rishi << ", "; } std::cout << "\n";

    for (const auto& devata : hymn.getDevatas()){
        std::cout <<"Devatas:" << devata << ", "; } std::cout << "\n";

    for (const auto& category : hymn.getCategories()){
        std::cout <<"Categories:" << category << ", "; } std::cout << "\n\n";

    for (const auto& verse : hymn.getVerses()) { //For every verse in Hymn, output Verse #, Dev, IAST, ENG.
    

        std::cout << "[Verse " << verse.getVerseNumber() << "]\n";
        std::cout << "DEV: " << verse.getDev() << "\n";
        std::cout << "IAST: " << verse.getIAST() << "\n";
        std::cout << "ENG: " << verse.getENG() << "\n\n";
        //Split Stream for DEV Words Only
        std::cout << "DEV Words:\n";
        for (const auto& w : verse.getDevWords()) {
            std::cout << "[" << w.getText() << "] ";
        }
        std::cout << "\n\n";
        //Split Stream for IAST Words Only
        std::cout << "IAST Words:\n";
        for (const auto& w : verse.getIASTWords()) {
            std::cout << "[" << w.getText() << "] ";
        }
        std::cout << "\n\n";

        //Debug Dev Letters
        std::cout << "DEV Letters:\n";
        for (const auto& w : verse.getDevWords()) {
            std::cout << w.getText() << " → ";
        for (const auto& l : w.getLetters()) {
            std::cout << l.getValue();

        if (l.getSwaraType().has_value())
            std::cout << "(" << l.getSwaraType().value() << ") ";

        else
            std::cout << "(none) ";
            }
            std::cout << "\n";
        }

        //Debug Vowel Weight
        std::cout << "\nSyllables:\n\n";

        for (const auto& w : verse.getDevWords()) {
            std::cout << w.getText() << " → ";

            for (const auto& s : w.getSyllables()) {

                std::cout << "[";
                
                for (const auto& l : s.getOnset())
                    std::cout << l.getValue();

                std::cout << "|";

                std::cout << s.getNucleus().getValue();

                std::cout << "|";

                for (const auto& l : s.getCoda())
                    std::cout << l.getValue();

                std::cout << ":" << s.getWeight() << "] ";
            }

            std::cout << "\n";
        }

        /*Letter Frequency Goes Here
        

        */
    }

    return 0;
}