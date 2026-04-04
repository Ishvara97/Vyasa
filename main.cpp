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

	Hymn hymn = parseHymn("Hymns/[Hymn 10.125].txt"); //Parse Hymn on File

    std::cout << "Mandala: " << hymn.getMandala() << "\n"; //Output Mandala#
    std::cout << "Sukta: " << hymn.getSukta() << "\n\n"; //Output Sukta#
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
    }

    return 0;
}