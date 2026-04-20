#include <fstream>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <windows.h>
#include "src/CleanUp.h"
#include "src/Parser.h"
#include "PoemStructures.h"
#include "src/analysis.h"
#include "json.hpp"
#include "src/jsonserialization.h"

namespace {
// Reconstruct a syllable as plain text for console output.
std::string syllableToText(const Syllable& syllable) {
    std::string text;

    for (const auto& letter : syllable.getOnset()) {
        text += letter.getValue();
    }

    text += syllable.getNucleus().getValue();

    for (const auto& letter : syllable.getCoda()) {
        text += letter.getValue();
    }

    return text;
}

// Replace characters that are invalid in Windows file names.
std::string sanitizeFilename(std::string value) {
    for (char& ch : value) {
        switch (ch) {
            case '<':
            case '>':
            case ':':
            case '"':
            case '/':
            case '\\':
            case '|':
            case '?':
            case '*':
                ch = '_';
                break;
            default:
                break;
        }
    }

    return value;
}

// Use the input file stem as the export base so variants like "Svaras" stay distinct.
std::string getExportBaseName(const std::string& sourcePath) {
    return sanitizeFilename(std::filesystem::path(sourcePath).stem().string());
}

// Prevent collisions when multiple input files would otherwise export to the same base name.
std::string makeUniqueExportBaseName(
    const std::string& sourcePath,
    std::unordered_set<std::string>& usedNames) {
    std::string baseName = getExportBaseName(sourcePath);

    if (baseName.empty()) {
        baseName = "Hymn";
    }

    std::string candidate = baseName;
    int suffix = 2;
    while (usedNames.find(candidate) != usedNames.end()) {
        candidate = baseName + "_" + std::to_string(suffix);
        ++suffix;
    }

    usedNames.insert(candidate);
    return candidate;
}

struct ParsedHymn {
    std::string sourcePath;
    Hymn hymn;
    std::string exportBaseName;
};

void printExportMessage(
    const std::string& sourcePath,
    const std::string& jsonPath,
    const std::string& csvPath,
    const std::string& analysisPath,
    const std::string& sandhiPath,
    const std::string& similarityPath) {
    std::cout << "\nExports for " << sourcePath << ":\n";
    std::cout << "  JSON: " << jsonPath << "\n";
    std::cout << "  CSV: " << csvPath << "\n";
    std::cout << "  Analysis CSV: " << analysisPath << "\n";
    std::cout << "  Sandhi CSV: " << sandhiPath << "\n";
    std::cout << "  Similarity CSV: " << similarityPath << "\n";
}
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::vector<ParsedHymn> hymns;
    std::unordered_set<std::string> usedExportBaseNames;
    int filenumbers = 0;

    std::cout << "Enter number of hymn files to input\n";
    std::cin >> filenumbers;

    if (!std::cin || filenumbers <= 0) {
        std::cerr << "Invalid number of hymn files.\n";
        return 1;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::filesystem::create_directories("HymnExports");

    for (int i = 0; i < filenumbers; i++) {
        std::string filename;
        std::cout << "Enter path to hymn file: ";
        std::getline(std::cin, filename);

        if (filename.empty()) {
            std::cerr << "Empty file path provided.\n";
            return 1;
        }

        Hymn hymn = parseHymn(filename);
        hymns.push_back({filename, hymn, makeUniqueExportBaseName(filename, usedExportBaseNames)});
    }

    for (auto& parsed : hymns) {
        Hymn& hymn = parsed.hymn;
        //Set Verse Meters
        for (auto& verse : hymn.getVersesMutable()) {
            verse.setMeter(detectVerseMeter(verse)); 
        }

    std::cout << "\n========================================\n";
    std::cout << "Source: " << parsed.sourcePath << "\n";
    std::cout << "Export Base: " << parsed.exportBaseName << "\n";
    std::cout << "========================================\n";

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
        //verse.setMeter(detectVerseMeter(verse));
        std::cout << "[Verse " << verse.getVerseNumber() << "]\n";
        std::cout << "DEV: " << verse.getDev() << "\n";
        std::cout << "IAST: " << verse.getIAST() << "\n";
        std::cout << "ENG: " << verse.getENG() << "\n";
        std::cout << "Verse " << verse.getVerseNumber()
          << " pada counts: "
          << formatPadaCounts(getPadaSyllableCounts(verse))
          << "\n";
        std::cout << "Meter: " << verse.getMeter() <<"\n\n";


        std::cout << "DEV Words:\n";
        for (const auto& word : verse.getDevWords()) {
            std::cout << "[" << word.getText() << "] ";
        }
        std::cout << "\n\n";

        std::cout << "IAST Words:\n";
        for (const auto& word : verse.getIASTWords()) {
            std::cout << "[" << word.getText() << "] ";
        }
        std::cout << "\n\n";

        std::cout << "DEV Letters:\n";
        for (const auto& word : verse.getDevWords()) {
            std::cout << word.getText() << " -> ";
            for (const auto& letter : word.getLetters()) {
                std::cout << letter.getValue();
                if (letter.getSwaraType().has_value()) {
                    std::cout << "(" << letter.getSwaraType().value() << ") ";
                } else {
                    std::cout << "(none) ";
                }
            }
            std::cout << "\n";
        }

        std::cout << "\nSyllables:\n\n";
        for (const auto& word : verse.getDevWords()) {
            std::cout << word.getText() << " -> ";

            for (const auto& syllable : word.getSyllables()) {
                std::cout << "[" << lettersToString(syllable.getOnset())
                          << "|" << syllable.getNucleus().getValue()
                          << "|" << lettersToString(syllable.getCoda())
                          << ":" << syllable.getWeight() << "] ";
            }

            std::cout << "\n";
        }

        std::cout << "\nAligned Syllables:\n";
        for (const auto& word : verse.getDevWords()) {
            std::cout << word.getText() << " -> " << word.getAlignedIAST() << "\n";
            std::cout << "Alignment size: " << word.getAlignment().size() << "\n";

            for (const auto& alignment : word.getAlignment()) {
                std::cout << "[DEV: " << syllableToText(alignment.dev)
                          << " <-> IAST: " << syllableToText(alignment.iast) << "] ";
            }

            std::cout << "\n\n";
        }

        const auto letterFreq = getLetterFrequency(verse);
        std::cout << "Letter Frequency:\n";
        for (const auto& [key, value] : letterFreq) {
            std::cout << key << ": " << value << "\n";
        }

        const auto swaraFreq = getSwaraFrequency(verse);
        std::cout << "\nSwara Frequency:\n";
        for (const auto& [key, value] : swaraFreq) {
            std::cout << key << ": " << value << "\n";
        }

        const auto pattern = getSyllablePattern(verse);
        std::cout << "\nSyllable Pattern:\n";
        for (const auto& item : pattern) {
            std::cout << item << " ";
        }

        std::cout << "\n\nPhoneme Class Frequency:\n";
        const auto phonemeFreq = getPhonemeClassFrequency(verse);
        for (const auto& [key, value] : phonemeFreq) {
            std::cout << key << ": " << value << "\n";
        }

        std::cout << "\n";
    }

    const auto hymnLetterFreq = getHymnLetterFrequency(hymn);
    std::cout << "\nHYMN LETTER FREQUENCY\n";
    for (const auto& [key, value] : hymnLetterFreq) {
        std::cout << key << ": " << value << "\n";
    }

    const auto hymnSwaraFreq = getHymnSwaraFrequency(hymn);
    std::cout << "\nHYMN SWARA FREQUENCY\n";
    for (const auto& [key, value] : hymnSwaraFreq) {
        std::cout << key << ": " << value << "\n";
    }

    const auto hymnPhonemeFreq = getHymnPhonemeClassFrequency(hymn);
    std::cout << "\nHYMN PHONEME CLASS FREQUENCY\n";
    for (const auto& [key, value] : hymnPhonemeFreq) {
        std::cout << key << ": " << value << "\n";
    }

    // Export the full structured hymn plus two CSV views for spreadsheet work.
    const std::string jsonPath = "HymnExports/" + parsed.exportBaseName + ".json";
    const std::string csvPath = "HymnExports/" + parsed.exportBaseName + ".csv";
    const std::string analysisPath = "HymnExports/" + parsed.exportBaseName + "_Analysis.csv";
    const std::string sandhiPath = "HymnExports/" + parsed.exportBaseName + "_Sandhi.csv";
    const std::string similarityPath = "HymnExports/" + parsed.exportBaseName + "_Similarity.csv";
    std::ofstream out(jsonPath);
    out << hymnToJsonString(hymn);

    exportFullCSV(hymn, csvPath);
    exportHymnAnalysisCSV(hymn, analysisPath);
    exportSandhiCSV(hymn, sandhiPath);
    exportVerseSimilarityCSV(hymn, similarityPath);
    printExportMessage(parsed.sourcePath, jsonPath, csvPath, analysisPath, sandhiPath, similarityPath);
    }

    return 0;
}
