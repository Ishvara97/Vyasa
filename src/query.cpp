#include "query.h"

#include "CleanUp.h"
#include "analysis.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <map>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>

namespace {
enum class SearchKind {
    VerseContains,
    WordContains,
    Prefix,
    Suffix,
    RegexPattern,
    PhonemeClass,
    Manner,
    ClusterLength
};

struct SearchRequest {
    SearchKind kind = SearchKind::WordContains;
    std::string value;
    int number = 0;
};

struct SearchMatch {
    int hymnIndex = 0;
    std::string hymnLabel;
    int verseNumber = 0;
    std::string devWord;
    std::string iastWord;
    std::string devVerse;
    std::string iastVerse;
    std::string detail;
};

struct VerseSelection {
    const LoadedHymnRecord* hymnRecord = nullptr;
    const Verse* verse = nullptr;
};

std::string trim(std::string value) {
    auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](unsigned char ch) {
        return !isSpace(ch);
    }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [&](unsigned char ch) {
        return !isSpace(ch);
    }).base(), value.end());
    return value;
}

std::string toLowerAscii(std::string value) {
    for (char& ch : value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

std::string stripOuterQuotes(std::string value) {
    value = trim(value);
    while (!value.empty()) {
        const char front = value.front();
        const char back = value.back();
        const bool paired =
            (front == '"' && back == '"') ||
            (front == '\'' && back == '\'') ||
            (front == '`' && back == '`');
        if (!paired) {
            break;
        }
        value = trim(value.substr(1, value.size() - 2));
    }
    return value;
}

std::string stripTrailingPunctuation(std::string value) {
    while (!value.empty()) {
        const unsigned char ch = static_cast<unsigned char>(value.back());
        if (std::ispunct(ch) == 0 || value.back() == ']' || value.back() == ')') {
            break;
        }
        value.pop_back();
    }
    return trim(value);
}

std::string normalizeQueryValue(std::string value) {
    value = stripOuterQuotes(trim(value));
    value = stripTrailingPunctuation(value);

    if (value.size() >= 1 && value.front() == '-') {
        value.erase(value.begin());
        value = trim(value);
    }

    const std::string lowered = toLowerAscii(value);
    if (lowered.rfind("particular ", 0) == 0) {
        value = trim(value.substr(std::string("particular ").size()));
    }

    return value;
}

bool containsText(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) {
        return false;
    }

    return toLowerAscii(haystack).find(toLowerAscii(needle)) != std::string::npos ||
           haystack.find(needle) != std::string::npos;
}

bool startsWithText(const std::string& haystack, const std::string& needle) {
    if (needle.empty() || haystack.size() < needle.size()) {
        return false;
    }

    return haystack.rfind(needle, 0) == 0 ||
           toLowerAscii(haystack).rfind(toLowerAscii(needle), 0) == 0;
}

bool endsWithText(const std::string& haystack, const std::string& needle) {
    if (needle.empty() || haystack.size() < needle.size()) {
        return false;
    }

    return haystack.compare(haystack.size() - needle.size(), needle.size(), needle) == 0 ||
           toLowerAscii(haystack).compare(
               haystack.size() - needle.size(),
               needle.size(),
               toLowerAscii(needle)) == 0;
}

std::optional<int> parsePositiveInt(const std::string& value) {
    const std::string cleaned = trim(value);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    for (char ch : cleaned) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return std::nullopt;
        }
    }

    return std::stoi(cleaned);
}

std::string getWordIAST(const Word& word) {
    if (!word.getUnderlyingText().empty()) {
        return word.getUnderlyingText();
    }
    if (!word.getAlignedIASTUnderlying().empty()) {
        return word.getAlignedIASTUnderlying();
    }
    if (!word.getAlignedIAST().empty()) {
        return word.getAlignedIAST();
    }
    return word.getText();
}

std::string getWordDevSurface(const Word& word) {
    if (!word.getUnderlyingDevText().empty()) {
        return word.getUnderlyingDevText();
    }
    return word.getText();
}

std::string describeSearchKind(const SearchRequest& request) {
    switch (request.kind) {
        case SearchKind::VerseContains:
            return "verse text contains \"" + request.value + "\"";
        case SearchKind::WordContains:
            return "word contains \"" + request.value + "\"";
        case SearchKind::Prefix:
            return "word begins with \"" + request.value + "\"";
        case SearchKind::Suffix:
            return "word ends with \"" + request.value + "\"";
        case SearchKind::RegexPattern:
            return "vowel/span regex \"" + request.value + "\"";
        case SearchKind::PhonemeClass:
            return "phoneme class \"" + request.value + "\"";
        case SearchKind::Manner:
            return "manner \"" + request.value + "\"";
        case SearchKind::ClusterLength:
            return "consonant cluster length >= " + std::to_string(request.number);
    }

    return request.value;
}

std::optional<SearchRequest> parseSearchRequest(const std::string& query) {
    const std::string cleaned = trim(query);
    const std::string lowered = toLowerAscii(cleaned);

    auto extractAfter = [&](const std::string& marker) -> std::string {
        const size_t pos = lowered.find(marker);
        if (pos == std::string::npos) {
            return "";
        }
        return normalizeQueryValue(cleaned.substr(pos + marker.size()));
    };

    if (lowered.find("words beginning with") != std::string::npos ||
        lowered.find("words starting with") != std::string::npos ||
        lowered.find("beginning with") != std::string::npos ||
        lowered.find("starting with") != std::string::npos) {
        std::string value = extractAfter("words beginning with");
        if (value.empty()) {
            value = extractAfter("words starting with");
        }
        if (value.empty()) {
            value = extractAfter("beginning with");
        }
        if (value.empty()) {
            value = extractAfter("starting with");
        }
        return SearchRequest{SearchKind::Prefix, value, 0};
    }

    if (lowered.find("words ending with") != std::string::npos ||
        lowered.find("ending with") != std::string::npos) {
        std::string value = extractAfter("words ending with");
        if (value.empty()) {
            value = extractAfter("ending with");
        }
        return SearchRequest{SearchKind::Suffix, value, 0};
    }

    if (lowered.find("consonant clusters of length >=") != std::string::npos) {
        const std::string value = extractAfter("consonant clusters of length >=");
        if (auto number = parsePositiveInt(value); number.has_value()) {
            return SearchRequest{SearchKind::ClusterLength, "", number.value()};
        }
        return std::nullopt;
    }

    if (lowered.find("phoneme class") != std::string::npos) {
        return SearchRequest{SearchKind::PhonemeClass, extractAfter("phoneme class"), 0};
    }

    if (lowered.find("manner of articulation") != std::string::npos) {
        return SearchRequest{SearchKind::Manner, extractAfter("manner of articulation"), 0};
    }

    if (lowered.find("vowel span patterns") != std::string::npos) {
        std::string value = extractAfter("vowel span patterns");
        if (value.empty()) {
            const size_t patternPos = lowered.find("pattern");
            if (patternPos != std::string::npos) {
                value = normalizeQueryValue(cleaned.substr(patternPos + std::string("patterns").size()));
            }
        }
        return SearchRequest{SearchKind::RegexPattern, value, 0};
    }

    if (lowered.find("find all verses containing") != std::string::npos) {
        return SearchRequest{
            SearchKind::VerseContains,
            extractAfter("find all verses containing"),
            0};
    }

    if (lowered.find("find all occurrences of") != std::string::npos) {
        return SearchRequest{
            SearchKind::WordContains,
            extractAfter("find all occurrences of"),
            0};
    }

    if (lowered.find("find words containing") != std::string::npos) {
        return SearchRequest{
            SearchKind::WordContains,
            extractAfter("find words containing"),
            0};
    }

    if (lowered.find("find verses containing") != std::string::npos) {
        return SearchRequest{
            SearchKind::VerseContains,
            extractAfter("find verses containing"),
            0};
    }

    if (lowered.find("containing") != std::string::npos) {
        return SearchRequest{SearchKind::WordContains, extractAfter("containing"), 0};
    }

    return SearchRequest{SearchKind::WordContains, normalizeQueryValue(cleaned), 0};
}

bool wordMatchesPhonemeClass(const Word& word, const std::string& queryValue) {
    const std::string loweredQuery = toLowerAscii(trim(queryValue));
    for (const auto& letter : word.getLetters()) {
        if (toLowerAscii(phonemeClassToString(letter.getPhoneme())) == loweredQuery) {
            return true;
        }
    }
    return false;
}

bool wordMatchesManner(const Word& word, const std::string& queryValue) {
    const std::string loweredQuery = toLowerAscii(trim(queryValue));

    for (const auto& letter : word.getLetters()) {
        const auto phoneme = letter.getPhoneme();
        if (loweredQuery == "stops" || loweredQuery == "stop") {
            if (phoneme.consonantClass == ConsonantClass::Velar ||
                phoneme.consonantClass == ConsonantClass::Palatal ||
                phoneme.consonantClass == ConsonantClass::Retroflex ||
                phoneme.consonantClass == ConsonantClass::Dental ||
                phoneme.consonantClass == ConsonantClass::Labial) {
                return true;
            }
        } else if ((loweredQuery == "nasals" || loweredQuery == "nasal") &&
                   phoneme.consonantClass == ConsonantClass::Nasal) {
            return true;
        } else if ((loweredQuery == "sibilants" || loweredQuery == "sibilant") &&
                   phoneme.consonantClass == ConsonantClass::Sibilant) {
            return true;
        } else if ((loweredQuery == "semivowels" || loweredQuery == "semivowel") &&
                   phoneme.consonantClass == ConsonantClass::Semivowel) {
            return true;
        } else if ((loweredQuery == "liquids" || loweredQuery == "liquid") &&
                   phoneme.consonantClass == ConsonantClass::Liquid) {
            return true;
        } else if ((loweredQuery == "aspirates" || loweredQuery == "aspirate") &&
                   phoneme.consonantClass == ConsonantClass::Aspirate) {
            return true;
        } else if ((loweredQuery == "vowels" || loweredQuery == "vowel") &&
                   phoneme.type == PhonemeType::Vowel) {
            return true;
        } else if ((loweredQuery == "consonants" || loweredQuery == "consonant") &&
                   phoneme.type == PhonemeType::Consonant) {
            return true;
        }
    }

    return false;
}

int longestConsonantClusterLength(const Word& word) {
    int best = 0;
    int current = 0;

    for (const auto& unit : splitIAST(getWordIAST(word))) {
        if (unit.empty() || isIgnorableSymbol(unit)) {
            current = 0;
            continue;
        }

        if (isIASTVowel(unit)) {
            current = 0;
            continue;
        }

        ++current;
        best = std::max(best, current);
    }

    return best;
}

bool wordMatchesRegex(const Word& word, const std::regex& pattern) {
    return std::regex_search(getWordIAST(word), pattern) ||
           std::regex_search(getWordDevSurface(word), pattern) ||
           std::regex_search(word.getText(), pattern);
}

bool wordMatchesSearch(const Word& word, const SearchRequest& request, std::string& detail) {
    const std::string dev = getWordDevSurface(word);
    const std::string iast = getWordIAST(word);

    switch (request.kind) {
        case SearchKind::WordContains:
            if (containsText(dev, request.value) || containsText(iast, request.value)) {
                detail = "matched word text";
                return true;
            }
            return false;
        case SearchKind::Prefix:
            if (startsWithText(dev, request.value) || startsWithText(iast, request.value)) {
                detail = "matched prefix";
                return true;
            }
            return false;
        case SearchKind::Suffix:
            if (endsWithText(dev, request.value) || endsWithText(iast, request.value)) {
                detail = "matched suffix";
                return true;
            }
            return false;
        case SearchKind::RegexPattern:
            try {
                const std::regex pattern(request.value, std::regex_constants::ECMAScript);
                if (wordMatchesRegex(word, pattern)) {
                    detail = "matched regex pattern";
                    return true;
                }
            } catch (const std::regex_error&) {
                detail = "invalid regex pattern";
            }
            return false;
        case SearchKind::PhonemeClass:
            if (wordMatchesPhonemeClass(word, request.value)) {
                detail = "matched phoneme class";
                return true;
            }
            return false;
        case SearchKind::Manner:
            if (wordMatchesManner(word, request.value)) {
                detail = "matched manner of articulation";
                return true;
            }
            return false;
        case SearchKind::ClusterLength: {
            const int best = longestConsonantClusterLength(word);
            if (best >= request.number) {
                detail = "cluster length " + std::to_string(best);
                return true;
            }
            return false;
        }
        case SearchKind::VerseContains:
            return false;
    }

    return false;
}

std::vector<SearchMatch> executeSearch(
    const std::vector<LoadedHymnRecord>& hymns,
    const SearchRequest& request,
    std::string* errorMessage = nullptr) {
    std::vector<SearchMatch> matches;
    bool regexFailed = false;

    for (const auto& record : hymns) {
        if (record.hymn == nullptr) {
            continue;
        }

        for (const auto& verse : record.hymn->getVerses()) {
            if (request.kind == SearchKind::VerseContains) {
                if (containsText(verse.getDev(), request.value) ||
                    containsText(verse.getIAST(), request.value) ||
                    containsText(verse.getENG(), request.value)) {
                    matches.push_back({
                        record.index,
                        record.exportBaseName,
                        verse.getVerseNumber(),
                        "",
                        "",
                        verse.getDev(),
                        verse.getIAST(),
                        "matched verse text"
                    });
                }
                continue;
            }

            for (size_t i = 0; i < verse.getDevWords().size(); ++i) {
                const Word& word = verse.getDevWords()[i];
                std::string detail;
                if (!wordMatchesSearch(word, request, detail)) {
                    if (request.kind == SearchKind::RegexPattern && detail == "invalid regex pattern") {
                        regexFailed = true;
                    }
                    continue;
                }

                std::string iastWord;
                if (i < verse.getIASTWords().size()) {
                    iastWord = getWordIAST(verse.getIASTWords()[i]);
                } else {
                    iastWord = getWordIAST(word);
                }

                matches.push_back({
                    record.index,
                    record.exportBaseName,
                    verse.getVerseNumber(),
                    getWordDevSurface(word),
                    iastWord,
                    verse.getDev(),
                    verse.getIAST(),
                    detail
                });
            }
        }
    }

    if (regexFailed && errorMessage != nullptr) {
        *errorMessage = "The regex pattern could not be compiled. Use a valid ECMAScript regex such as śr[aā]ddh.";
    }

    return matches;
}

std::map<std::string, int> buildMeterPatternProfileForVerse(const Verse& verse) {
    std::map<std::string, int> profile;
    const auto pattern = getSyllablePattern(verse);
    for (size_t i = 0; i < pattern.size(); ++i) {
        profile[std::to_string(i + 1) + ":" + pattern[i]]++;
    }
    return profile;
}

std::map<std::string, int> buildMeterPatternProfileForHymn(const Hymn& hymn) {
    std::map<std::string, int> profile;
    for (const auto& verse : hymn.getVerses()) {
        const auto verseProfile = buildMeterPatternProfileForVerse(verse);
        for (const auto& [key, value] : verseProfile) {
            profile[key] += value;
        }
    }
    return profile;
}

SimilarityScore compareIntMaps(
    const std::map<std::string, int>& left,
    const std::map<std::string, int>& right) {
    const auto [leftVector, rightVector] = alignFeatureVectors(left, right);
    return buildSimilarityScore(leftVector, rightVector);
}

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

std::optional<VerseSelection> resolveVerseReference(
    const std::vector<LoadedHymnRecord>& hymns,
    const std::string& reference) {
    const size_t colonPos = reference.find(':');
    if (colonPos == std::string::npos) {
        return std::nullopt;
    }

    const auto hymnIndex = parsePositiveInt(reference.substr(0, colonPos));
    const auto verseNumber = parsePositiveInt(reference.substr(colonPos + 1));
    if (!hymnIndex.has_value() || !verseNumber.has_value()) {
        return std::nullopt;
    }

    for (const auto& record : hymns) {
        if (record.index != hymnIndex.value() || record.hymn == nullptr) {
            continue;
        }

        for (const auto& verse : record.hymn->getVerses()) {
            if (verse.getVerseNumber() == verseNumber.value()) {
                return VerseSelection{&record, &verse};
            }
        }
    }

    return std::nullopt;
}

const LoadedHymnRecord* resolveHymnReference(
    const std::vector<LoadedHymnRecord>& hymns,
    const std::string& reference) {
    const auto hymnIndex = parsePositiveInt(reference);
    if (!hymnIndex.has_value()) {
        return nullptr;
    }

    for (const auto& record : hymns) {
        if (record.index == hymnIndex.value()) {
            return &record;
        }
    }

    return nullptr;
}
}

std::string buildLoadedHymnSummary(const std::vector<LoadedHymnRecord>& hymns) {
    std::ostringstream out;
    out << "Loaded hymns:\n";
    for (const auto& record : hymns) {
        if (record.hymn == nullptr) {
            continue;
        }
        out << "  [" << record.index << "] "
            << record.exportBaseName
            << " (" << record.sourcePath << ")"
            << " verses=" << record.hymn->getVerses().size() << "\n";
    }
    return out.str();
}

std::string buildInteractiveActionPrompt() {
    std::ostringstream out;
    out << "Request one of these actions next:\n";
    out << "  search         Search for information in a hymn or across loaded hymns\n";
    out << "  compare verses Compare two or more verses across hymns using hymnIndex:verseNumber\n";
    out << "  compare hymns  Compare two or more hymns overall using hymn indices\n";
    out << "  compare query  Compare how one search pattern behaves across loaded hymns\n";
    out << "  list           Show loaded hymns again\n";
    out << "  help           Show command help\n";
    out << "  exit           Terminate the program when requested\n";
    return out.str();
}

std::string runSearchQuery(const std::vector<LoadedHymnRecord>& hymns, const std::string& query) {
    const auto request = parseSearchRequest(query);
    if (!request.has_value() || (request->value.empty() && request->kind != SearchKind::ClusterLength)) {
        return std::string("Could not understand the search request. Try examples like:\n") +
               "  find all verses containing " + u8"अग्नि" + "\n" +
               "  find all occurrences of indra\n" +
               "  find words beginning with " + u8"प्र" + "\n" +
               "  find words ending with -" + u8"त्व" + "\n" +
               "  find vowel span patterns śr[aā]ddh\n" +
               "  find verse/words containing particular phoneme class Dental\n" +
               "  find verse/words containing particular manner of articulation nasals\n" +
               "  find words containing kṣ\n" +
               "  find consonant clusters of length >= 3\n";
    }

    std::string errorMessage;
    const auto matches = executeSearch(hymns, request.value(), &errorMessage);
    if (!errorMessage.empty()) {
        return errorMessage + "\n";
    }

    std::map<int, int> hymnMatchCounts;
    std::map<int, std::set<int>> hymnVerseCounts;

    for (const auto& match : matches) {
        hymnMatchCounts[match.hymnIndex]++;
        hymnVerseCounts[match.hymnIndex].insert(match.verseNumber);
    }

    std::ostringstream out;
    out << "Search results for " << describeSearchKind(request.value()) << "\n";

    if (matches.empty()) {
        out << "No matches found.\n";
        return out.str();
    }

    out << "Summary by hymn:\n";
    for (const auto& record : hymns) {
        out << "  [" << record.index << "] " << record.exportBaseName
            << " matches=" << hymnMatchCounts[record.index]
            << " verses=" << hymnVerseCounts[record.index].size() << "\n";
    }

    out << "Detailed matches:\n";
    for (const auto& match : matches) {
        out << "  [" << match.hymnIndex << "] " << match.hymnLabel
            << " Verse " << match.verseNumber;
        if (!match.devWord.empty() || !match.iastWord.empty()) {
            out << " DEV=\"" << match.devWord << "\"";
            if (!match.iastWord.empty()) {
                out << " IAST=\"" << match.iastWord << "\"";
            }
        }
        out << " (" << match.detail << ")\n";
        out << "    Verse DEV: " << match.devVerse << "\n";
        out << "    Verse IAST: " << match.iastVerse << "\n";
    }

    return out.str();
}

std::string compareSelectedVerses(
    const std::vector<LoadedHymnRecord>& hymns,
    const std::string& references) {
    std::vector<VerseSelection> selected;

    for (const auto& reference : splitCommaSeparated(references)) {
        const auto resolved = resolveVerseReference(hymns, reference);
        if (!resolved.has_value()) {
            return "Invalid verse reference: " + reference +
                   "\nUse hymnIndex:verseNumber, e.g. 1:4,2:6";
        }
        selected.push_back(resolved.value());
    }

    if (selected.size() < 2) {
        return "Provide at least two verse references, e.g. 1:4,2:6";
    }

    std::ostringstream out;
    out << std::fixed << std::setprecision(6);
    out << "Verse comparison results\n";

    for (size_t i = 0; i < selected.size(); ++i) {
        for (size_t j = i + 1; j < selected.size(); ++j) {
            const auto phoneme = compareIntMaps(
                getPhonemeClassFrequency(*selected[i].verse),
                getPhonemeClassFrequency(*selected[j].verse));
            const auto swara = compareIntMaps(
                getSwaraFrequency(*selected[i].verse),
                getSwaraFrequency(*selected[j].verse));
            const auto meter = compareIntMaps(
                buildMeterPatternProfileForVerse(*selected[i].verse),
                buildMeterPatternProfileForVerse(*selected[j].verse));

            out << "  " << selected[i].hymnRecord->exportBaseName << ":" << selected[i].verse->getVerseNumber()
                << " vs "
                << selected[j].hymnRecord->exportBaseName << ":" << selected[j].verse->getVerseNumber()
                << "\n";
            out << "    phoneme cosine=" << phoneme.cosineSimilarity
                << " confidence=" << phoneme.confidence << "\n";
            out << "    swara cosine=" << swara.cosineSimilarity
                << " confidence=" << swara.confidence << "\n";
            out << "    meter cosine=" << meter.cosineSimilarity
                << " confidence=" << meter.confidence << "\n";
        }
    }

    return out.str();
}

std::string compareSelectedHymns(
    const std::vector<LoadedHymnRecord>& hymns,
    const std::string& references) {
    std::vector<const LoadedHymnRecord*> selected;
    for (const auto& reference : splitCommaSeparated(references)) {
        const LoadedHymnRecord* resolved = resolveHymnReference(hymns, reference);
        if (resolved == nullptr || resolved->hymn == nullptr) {
            return "Invalid hymn reference: " + reference + "\nUse hymn indices, e.g. 1,2";
        }
        selected.push_back(resolved);
    }

    if (selected.size() < 2) {
        return "Provide at least two hymn indices, e.g. 1,2";
    }

    std::ostringstream out;
    out << std::fixed << std::setprecision(6);
    out << "Hymn comparison results\n";

    for (size_t i = 0; i < selected.size(); ++i) {
        for (size_t j = i + 1; j < selected.size(); ++j) {
            const auto phoneme = compareIntMaps(
                getHymnPhonemeClassFrequency(*selected[i]->hymn),
                getHymnPhonemeClassFrequency(*selected[j]->hymn));
            const auto swara = compareIntMaps(
                getHymnSwaraFrequency(*selected[i]->hymn),
                getHymnSwaraFrequency(*selected[j]->hymn));
            const auto meter = compareIntMaps(
                buildMeterPatternProfileForHymn(*selected[i]->hymn),
                buildMeterPatternProfileForHymn(*selected[j]->hymn));

            out << "  [" << selected[i]->index << "] " << selected[i]->exportBaseName
                << " vs "
                << "[" << selected[j]->index << "] " << selected[j]->exportBaseName
                << "\n";
            out << "    verses=" << selected[i]->hymn->getVerses().size()
                << " vs " << selected[j]->hymn->getVerses().size() << "\n";
            out << "    phoneme cosine=" << phoneme.cosineSimilarity
                << " confidence=" << phoneme.confidence << "\n";
            out << "    swara cosine=" << swara.cosineSimilarity
                << " confidence=" << swara.confidence << "\n";
            out << "    meter cosine=" << meter.cosineSimilarity
                << " confidence=" << meter.confidence << "\n";
        }
    }

    return out.str();
}

std::string compareSearchQueryAcrossHymns(
    const std::vector<LoadedHymnRecord>& hymns,
    const std::string& query) {
    const auto request = parseSearchRequest(query);
    if (!request.has_value() || (request->value.empty() && request->kind != SearchKind::ClusterLength)) {
        return std::string("Could not understand the comparison query. Use a search-like query such as:\n") +
               "  find all occurrences of indra\n" +
               "  find words beginning with " + u8"प्र" + "\n" +
               "  find consonant clusters of length >= 3\n";
    }

    std::string errorMessage;
    const auto matches = executeSearch(hymns, request.value(), &errorMessage);
    if (!errorMessage.empty()) {
        return errorMessage + "\n";
    }

    std::map<int, int> hymnMatchCounts;
    std::map<int, std::set<int>> hymnVerseCounts;

    for (const auto& match : matches) {
        hymnMatchCounts[match.hymnIndex]++;
        hymnVerseCounts[match.hymnIndex].insert(match.verseNumber);
    }

    std::ostringstream out;
    out << "Comparison of query across loaded hymns: " << describeSearchKind(request.value()) << "\n";
    for (const auto& record : hymns) {
        const int matchCount = hymnMatchCounts[record.index];
        const int verseCount = static_cast<int>(hymnVerseCounts[record.index].size());
        const size_t totalVerses = record.hymn != nullptr ? record.hymn->getVerses().size() : 0;
        const double density =
            totalVerses == 0 ? 0.0 : static_cast<double>(verseCount) / static_cast<double>(totalVerses);

        out << "  [" << record.index << "] " << record.exportBaseName
            << " matches=" << matchCount
            << " matchedVerses=" << verseCount
            << " totalVerses=" << totalVerses
            << " verseCoverage=" << std::fixed << std::setprecision(3) << density
            << "\n";
    }

    if (!matches.empty()) {
        out << "Matched verse references:\n";
        for (const auto& match : matches) {
            out << "  [" << match.hymnIndex << "] " << match.hymnLabel
                << ":" << match.verseNumber;
            if (!match.devWord.empty()) {
                out << " DEV=\"" << match.devWord << "\"";
            }
            if (!match.iastWord.empty()) {
                out << " IAST=\"" << match.iastWord << "\"";
            }
            out << "\n";
        }
    } else {
        out << "No hymns matched the comparison query.\n";
    }

    return out.str();
}
