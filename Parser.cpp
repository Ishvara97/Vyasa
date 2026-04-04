#include "Parser.h"
#include "CleanUp.h"

#include <fstream>
#include <regex>

Hymn parseHymn(const std::string& filename) {
    Hymn hymn;

    // Read file
    std::ifstream file(filename);
    std::string text((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

    auto lines = splitLines(text);//Perform SplitLines on File (CleanUp)

    Verse currentVerse; //Create a Verse called currentVerse
    //Establish Regular Expression for Hymn Properties
    std::regex mandala_re("Mandala:\\s*(\\d+)");
    std::regex sukta_re("Sukta:\\s*(\\d+)");
    std::regex rishi_re("Rishis:\\s*(.*)");
    std::regex devata_re("Devatas:\\s*(.*)");
    std::regex category_re("Categories:\\s*(.*)");
    std::regex verse_re("\\[Verse\\s*(\\d+)\\]");
    //
    std::smatch match;
    //For every line in lines (CleanUp) match initial text (above) property to regex and place it in.
    for (const auto& line : lines) {

        if (std::regex_search(line, match, mandala_re)) { //e.g. if line starts with "Mandala:\\s*(\\d+)" then set that Mandala String ("1") as the Mandala Integer 1 for the Hymn
            hymn.setMandala(std::stoi(match[1]));
        }
        else if (std::regex_search(line, match, sukta_re)) {
            hymn.setSukta(std::stoi(match[1]));
        }
        else if (std::regex_search(line, match, rishi_re)) {
            hymn.addRishi(match[1]);
        }
        else if (std::regex_search(line, match, devata_re)) {
            hymn.addDevata(match[1]);
        }
        else if (std::regex_search(line, match, category_re)) {
            hymn.addCategory(match[1]);
        }
        else if (std::regex_search(line, match, verse_re)) {

            // Save previous verse if exists
            if (currentVerse.getVerseNumber() != 0) {
                hymn.addVerse(currentVerse);
                currentVerse = Verse();
            }

            currentVerse.setVerseNumber(std::stoi(match[1]));//Convert String to Integer
        }
        else if (line.rfind("DEV:", 0) == 0) { //When DEV: found, set current VerseDev to it, substring starting with 4th character on line.
            currentVerse.setDev(line.substr(4));
        }
        else if (line.rfind("IAST:", 0) == 0) {
            currentVerse.setIAST(line.substr(5)); //5th Character for IAST
        }
        else if (line.rfind("ENG:", 0) == 0) {
            currentVerse.setENG(line.substr(4)); //4th Character for ENG:
        }
    }

    // Add last verse
    if (currentVerse.getVerseNumber() != 0) {
        hymn.addVerse(currentVerse);
    }

    return hymn;
}
