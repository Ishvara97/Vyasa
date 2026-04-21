#include <fstream>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
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
#include "src/query.h"
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

std::string trim(std::string value);
std::string resolveHymnInputPath(const std::string& filename);
void printAvailableHymnFiles();
std::optional<std::string> promptSearchValue(
    const std::string& prompt,
    const std::string& helpText = "");
void printExportMessage(
    const std::string& sourcePath,
    const std::string& jsonPath,
    const std::string& csvPath,
    const std::string& analysisPath,
    const std::string& sandhiPath,
    const std::string& similarityPath);

std::vector<std::string> splitCommaSeparated(const std::string& value) {
    std::vector<std::string> parts;
    std::stringstream ss(value);
    std::string part;

    while (std::getline(ss, part, ',')) {
        part = trim(part);
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    return parts;
}

std::string trim(std::string value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::string toLowerAscii(std::string value) {
    for (char& ch : value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

bool isBackCommand(const std::string& value) {
    const std::string lowered = toLowerAscii(trim(value));
    return lowered == "back" || lowered == "cancel";
}

bool isExitCommand(const std::string& value) {
    const std::string lowered = toLowerAscii(trim(value));
    return lowered == "exit" || lowered == "quit" || lowered == "stop";
}

std::vector<std::string> getAvailableHymnFiles() {
    std::vector<std::string> files;
    const std::filesystem::path hymnFolder("Hymns");

    if (!std::filesystem::exists(hymnFolder)) {
        return files;
    }

    for (const auto& entry : std::filesystem::directory_iterator(hymnFolder)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (toLowerAscii(entry.path().extension().string()) == ".txt") {
            files.push_back(entry.path().filename().string());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::optional<std::string> resolveHymnFileSelection(
    const std::string& rawInput,
    const std::vector<std::string>& availableFiles) {
    const std::string cleaned = trim(rawInput);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    try {
        const int numericChoice = std::stoi(cleaned);
        if (numericChoice >= 1 && numericChoice <= static_cast<int>(availableFiles.size())) {
            return availableFiles[static_cast<size_t>(numericChoice - 1)];
        }
    } catch (const std::exception&) {
    }

    for (const auto& file : availableFiles) {
        if (toLowerAscii(file) == toLowerAscii(cleaned)) {
            return file;
        }
    }

    return cleaned;
}

void assignVerseMeters(Hymn& hymn) {
    for (auto& verse : hymn.getVersesMutable()) {
        verse.setMeter(detectVerseMeter(verse));
    }
}

void printParsedHymnDetails(const ParsedHymn& parsed) {
    const Hymn& hymn = parsed.hymn;
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
        std::cout << "[Verse " << verse.getVerseNumber() << "]\n";
        std::cout << "DEV: " << verse.getDev() << "\n";
        std::cout << "IAST: " << verse.getIAST() << "\n";
        std::cout << "ENG: " << verse.getENG() << "\n";
        std::cout << "Verse " << verse.getVerseNumber()
                  << " pada counts: "
                  << formatPadaCounts(getPadaSyllableCounts(verse))
                  << "\n";
        std::cout << "Meter: " << verse.getMeter() << "\n\n";

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
}

void exportParsedHymn(const ParsedHymn& parsed) {
    const Hymn& hymn = parsed.hymn;
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

std::vector<LoadedHymnRecord> buildLoadedHymnRecords(std::vector<ParsedHymn>& hymns) {
    std::vector<LoadedHymnRecord> loadedHymns;
    loadedHymns.reserve(hymns.size());
    for (size_t i = 0; i < hymns.size(); ++i) {
        loadedHymns.push_back({
            static_cast<int>(i + 1),
            hymns[i].sourcePath,
            hymns[i].exportBaseName,
            &hymns[i].hymn
        });
    }
    return loadedHymns;
}

std::optional<bool> addHymnToIndex(
    std::vector<ParsedHymn>& hymns,
    std::unordered_set<std::string>& usedExportBaseNames) {
    const auto availableHymnFiles = getAvailableHymnFiles();
    printAvailableHymnFiles();

    while (true) {
        const auto value = promptSearchValue(
            "Add hymn by listed number or file name (`back` to return): ",
            "Add hymn help:\n"
            "  Enter a listed number such as 1.\n"
            "  Or enter a file name such as Hymn 1.1.txt.\n"
            "  Type `back` to return.\n");
        if (!value.has_value()) {
            return std::nullopt;
        }
        if (value.value() == "__BACK__") {
            return false;
        }

        const auto resolvedSelection = resolveHymnFileSelection(value.value(), availableHymnFiles);
        if (!resolvedSelection.has_value()) {
            std::cout << "Empty file name provided.\n";
            continue;
        }

        const std::string sourcePath = resolveHymnInputPath(resolvedSelection.value());
        if (!std::filesystem::exists(sourcePath)) {
            std::cout << "File not found in Hymns: " << sourcePath << "\n";
            continue;
        }

        Hymn hymn = parseHymn(sourcePath);
        assignVerseMeters(hymn);
        hymns.push_back({sourcePath, hymn, makeUniqueExportBaseName(sourcePath, usedExportBaseNames)});
        printParsedHymnDetails(hymns.back());
        exportParsedHymn(hymns.back());
        std::cout << "Hymn added to loaded index.\n";
        return true;
    }
}

std::optional<bool> removeHymnsFromIndex(std::vector<ParsedHymn>& hymns) {
    if (hymns.empty()) {
        std::cout << "No loaded hymns to remove.\n";
        return false;
    }

    const auto loadedHymns = buildLoadedHymnRecords(hymns);
    std::cout << buildLoadedHymnSummary(loadedHymns);

    while (true) {
        const auto value = promptSearchValue(
            "Remove hymn indices, comma-separated (`back` to return): ",
            "Remove hymns help:\n"
            "  Use currently loaded hymn indices separated by commas.\n"
            "  Example: 2\n"
            "  Example: 1,3\n"
            "  Type `back` to return.\n");
        if (!value.has_value()) {
            return std::nullopt;
        }
        if (value.value() == "__BACK__") {
            return false;
        }

        std::vector<int> indices;
        bool valid = true;
        for (const auto& part : splitCommaSeparated(value.value())) {
            try {
                const int index = std::stoi(part);
                if (index < 1 || index > static_cast<int>(hymns.size())) {
                    valid = false;
                    break;
                }
                indices.push_back(index);
            } catch (const std::exception&) {
                valid = false;
                break;
            }
        }

        if (!valid || indices.empty()) {
            std::cout << "Invalid hymn indices.\n";
            continue;
        }

        std::sort(indices.begin(), indices.end());
        indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

        for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
            hymns.erase(hymns.begin() + (*it - 1));
        }

        std::cout << "Selected hymns removed from loaded index.\n";
        return true;
    }
}

std::string resolveHymnInputPath(const std::string& filename) {
    std::filesystem::path inputPath = trim(filename);
    if (!inputPath.has_parent_path()) {
        inputPath = std::filesystem::path("Hymns") / inputPath;
    }
    return inputPath.string();
}

void printAvailableHymnFiles() {
    const auto files = getAvailableHymnFiles();
    if (files.empty()) {
        std::cout << "No .txt files found in Hymns.\n";
        return;
    }

    std::cout << "Available hymn files in /Hymns:\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << files[i] << "\n";
    }
}

std::optional<std::string> promptLine(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);

    if (isExitCommand(input)) {
        return std::nullopt;
    }

    return trim(input);
}

std::optional<int> promptMenuSelection(
    const std::string& title,
    const std::vector<std::string>& options,
    bool allowBack = true) {
    while (true) {
        std::cout << "\n" << title << "\n";
        for (size_t i = 0; i < options.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << options[i] << "\n";
        }
        if (allowBack) {
            std::cout << "Type a number, or `back` to return.\n";
        } else {
            std::cout << "Type a number.\n";
        }

        const auto input = promptLine("Selection: ");
        if (!input.has_value()) {
            return std::nullopt;
        }

        if (allowBack && isBackCommand(input.value())) {
            return -1;
        }

        try {
            const int choice = std::stoi(input.value());
            if (choice >= 1 && choice <= static_cast<int>(options.size())) {
                return choice;
            }
        } catch (const std::exception&) {
        }

        std::cout << "Invalid selection.\n";
    }
}

std::optional<std::string> promptSearchValue(
    const std::string& prompt,
    const std::string& helpText) {
    while (true) {
        const auto value = promptLine(prompt);
        if (!value.has_value()) {
            return std::nullopt;
        }

        if (toLowerAscii(value.value()) == "help") {
            if (!helpText.empty()) {
                std::cout << "\n" << helpText;
            } else {
                std::cout << "No additional help is available for this step.\n";
            }
            continue;
        }

        if (isBackCommand(value.value())) {
            return std::string("__BACK__");
        }

        if (!value->empty()) {
            return value;
        }

        std::cout << "Input cannot be empty.\n";
    }
}

std::optional<std::string> buildStepByStepSearchQuery() {
    while (true) {
        const auto searchChoice = promptMenuSelection(
            "Search",
            {
                "Find verses containing text",
                "Find words containing text or letters",
                "Find all occurrences of text",
                "Find words beginning with a prefix",
                "Find words ending with a suffix",
                "Find vowel span pattern",
                "Find phoneme class",
                "Find manner of articulation",
                "Find consonant clusters by length"
            });

        if (!searchChoice.has_value()) {
            return std::nullopt;
        }
        if (searchChoice.value() == -1) {
            return std::string();
        }

        std::string query;
        if (searchChoice.value() == 1) {
            const auto value = promptSearchValue("Verse text to find (`back` to return). Accepted input: DEV, IAST, or Both: ");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            query = "find all verses containing " + value.value();
        } else if (searchChoice.value() == 2) {
            const auto value = promptSearchValue("Word text or letters to find (`back` to return). Accepted input: DEV, IAST, or Both: ");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            query = "find words containing " + value.value();
        } else if (searchChoice.value() == 3) {
            const auto value = promptSearchValue("Occurrence text to find (`back` to return). Accepted input: DEV, IAST, or Both: ");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            query = "find all occurrences of " + value.value();
        } else if (searchChoice.value() == 4) {
            const auto value = promptSearchValue("Prefix (`back` to return). Accepted input: DEV, IAST, or Both: ");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            query = "find words beginning with " + value.value();
        } else if (searchChoice.value() == 5) {
            const auto value = promptSearchValue("Suffix (`back` to return). Accepted input: DEV, IAST, or Both: ");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            query = "find words ending with -" + value.value();
        } else if (searchChoice.value() == 6) {
            const auto value = promptSearchValue("Regex / vowel span pattern (`back` to return). Accepted input: DEV, IAST, or Both; best results are usually IAST: ");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            query = "find vowel span patterns " + value.value();
        } else if (searchChoice.value() == 7) {
            const auto classChoice = promptMenuSelection(
                "Phoneme Class (selection only; no DEV/IAST text input at this step)",
                {
                    "Vowel",
                    "Velar",
                    "Palatal",
                    "Retroflex",
                    "Dental",
                    "Labial",
                    "Nasal",
                    "Semivowel",
                    "Liquid",
                    "Sibilant",
                    "Aspirate",
                    "Other"
                });
            if (!classChoice.has_value()) {
                return std::nullopt;
            }
            if (classChoice.value() == -1) {
                continue;
            }
            const std::vector<std::string> classes = {
                "Vowel", "Velar", "Palatal", "Retroflex", "Dental", "Labial",
                "Nasal", "Semivowel", "Liquid", "Sibilant", "Aspirate", "Other"
            };
            query = "find verse/words containing particular phoneme class " +
                    classes[static_cast<size_t>(classChoice.value() - 1)];
        } else if (searchChoice.value() == 8) {
            const auto mannerChoice = promptMenuSelection(
                "Manner Of Articulation (selection only; no DEV/IAST text input at this step)",
                {
                    "Stops",
                    "Nasals",
                    "Sibilants",
                    "Semivowels",
                    "Liquids",
                    "Aspirates",
                    "Vowels",
                    "Consonants"
                });
            if (!mannerChoice.has_value()) {
                return std::nullopt;
            }
            if (mannerChoice.value() == -1) {
                continue;
            }
            const std::vector<std::string> manners = {
                "stops", "nasals", "sibilants", "semivowels",
                "liquids", "aspirates", "vowels", "consonants"
            };
            query = "find verse/words containing particular manner of articulation " +
                    manners[static_cast<size_t>(mannerChoice.value() - 1)];
        } else if (searchChoice.value() == 9) {
            const auto value = promptSearchValue("Minimum cluster length (`back` to return). Input type: number only; analysis is IAST-oriented: ");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            query = "find consonant clusters of length >= " + value.value();
        }

        return query;
    }
}

std::optional<std::string> runStepByStepSearch(const std::vector<LoadedHymnRecord>& loadedHymns) {
    const auto query = buildStepByStepSearchQuery();
    if (!query.has_value()) {
        return std::nullopt;
    }
    if (query->empty()) {
        return std::string();
    }

    std::cout << "\n" << runSearchQuery(loadedHymns, query.value());
    return query;
}

std::optional<bool> runStepByStepCompare(const std::vector<LoadedHymnRecord>& loadedHymns) {
    while (true) {
        const auto compareChoice = promptMenuSelection(
            "Compare",
            {
                "Compare verses",
                "Compare hymns",
                "Compare one search pattern across hymns"
            });

        if (!compareChoice.has_value()) {
            return std::nullopt;
        }
        if (compareChoice.value() == -1) {
            return false;
        }

        if (compareChoice.value() == 1) {
            std::cout << buildLoadedHymnSummary(loadedHymns);
            const auto value = promptSearchValue(
                "Verse references as hymnIndex:verseNumber, comma-separated (`back` to return). Input type: selection only, not DEV/IAST text: ",
                "Compare verses help:\n"
                "  Use hymnIndex:verseNumber references separated by commas.\n"
                "  Example: 1:4,2:6\n"
                "  Example: 1:1,1:3,2:2\n"
                "  Type `back` to return to Compare.\n");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            std::cout << "\n" << compareSelectedVerses(loadedHymns, value.value());
            return true;
        }

        if (compareChoice.value() == 2) {
            std::cout << buildLoadedHymnSummary(loadedHymns);
            const auto value = promptSearchValue(
                "Hymn indices, comma-separated (`back` to return). Input type: selection only, not DEV/IAST text: ",
                "Compare hymns help:\n"
                "  Use hymn indices separated by commas.\n"
                "  Example: 1,2\n"
                "  Example: 1,2,3\n"
                "  Type `back` to return to Compare.\n");
            if (!value.has_value()) {
                return std::nullopt;
            }
            if (value.value() == "__BACK__") {
                continue;
            }
            std::cout << "\n" << compareSelectedHymns(loadedHymns, value.value());
            return true;
        }

        const auto builtQuery = buildStepByStepSearchQuery();
        if (!builtQuery.has_value()) {
            return std::nullopt;
        }
        if (builtQuery->empty()) {
            continue;
        }
        std::cout << "\n" << compareSearchQueryAcrossHymns(loadedHymns, builtQuery.value());
        return true;
    }
}

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

void printInteractiveHelp() {
    std::cout << "\n" << buildInteractiveActionPrompt();
    std::cout << "Use `back` to return from any submenu and `exit` to terminate the program.\n";
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
    const auto availableHymnFiles = getAvailableHymnFiles();
    printAvailableHymnFiles();

    for (int i = 0; i < filenumbers; i++) {
        while (true) {
            std::string filename;
            std::cout << "Enter File Name for " << (i + 1) << ": ";
            std::getline(std::cin, filename);

            if (isExitCommand(filename)) {
                std::cout << "Session terminated.\n";
                return 0;
            }

            const auto resolvedSelection = resolveHymnFileSelection(filename, availableHymnFiles);
            if (!resolvedSelection.has_value()) {
                std::cout << "Empty file name provided. Enter a listed number or file name.\n";
                continue;
            }

            const std::string sourcePath = resolveHymnInputPath(resolvedSelection.value());
            if (!std::filesystem::exists(sourcePath)) {
                std::cout << "File not found in Hymns: " << sourcePath << "\n";
                std::cout << "Enter a listed number or file name from /Hymns.\n";
                continue;
            }

            Hymn hymn = parseHymn(sourcePath);
            assignVerseMeters(hymn);
            hymns.push_back({sourcePath, hymn, makeUniqueExportBaseName(sourcePath, usedExportBaseNames)});
            break;
        }
    }

    for (const auto& parsed : hymns) {
        printParsedHymnDetails(parsed);
        exportParsedHymn(parsed);
    }

    std::vector<LoadedHymnRecord> loadedHymns = buildLoadedHymnRecords(hymns);

    std::cout << "\n" << buildLoadedHymnSummary(loadedHymns);
    std::cout << "\nUploads, parsing, and exports are complete.\n";
    std::cout << "Choose an action step by step to search or compare the loaded hymns.\n";
    std::cout << "The program will keep running until you request termination with `exit`, `quit`, or `stop`.\n";
    printInteractiveHelp();

    while (true) {
        const auto selection = promptMenuSelection(
            "Main Menu",
            {
                "Search",
                "Compare",
                "List loaded hymns",
                "Add hymn",
                "Remove hymn",
                "Help",
                "Exit"
            },
            false);

        if (!selection.has_value()) {
            std::cout << "Session terminated.\n";
            break;
        }

        if (selection.value() == 7) {
            std::cout << "Session terminated.\n";
            break;
        }

        if (selection.value() == 6) {
            printInteractiveHelp();
            continue;
        }

        if (selection.value() == 5) {
            const auto removed = removeHymnsFromIndex(hymns);
            if (!removed.has_value()) {
                std::cout << "Session terminated.\n";
                break;
            }
            if (removed.value()) {
                loadedHymns = buildLoadedHymnRecords(hymns);
                std::cout << buildLoadedHymnSummary(loadedHymns);
            }
            continue;
        }

        if (selection.value() == 4) {
            const auto added = addHymnToIndex(hymns, usedExportBaseNames);
            if (!added.has_value()) {
                std::cout << "Session terminated.\n";
                break;
            }
            if (added.value()) {
                loadedHymns = buildLoadedHymnRecords(hymns);
                std::cout << buildLoadedHymnSummary(loadedHymns);
            }
            continue;
        }

        if (selection.value() == 3) {
            std::cout << buildLoadedHymnSummary(loadedHymns);
            continue;
        }

        if (selection.value() == 1) {
            const auto result = runStepByStepSearch(loadedHymns);
            if (!result.has_value()) {
                std::cout << "Session terminated.\n";
                break;
            }
            continue;
        }

        const auto result = runStepByStepCompare(loadedHymns);
        if (!result.has_value()) {
            std::cout << "Session terminated.\n";
            break;
        }
    }

    return 0;
}
