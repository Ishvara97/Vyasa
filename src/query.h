#ifndef QUERY_H
#define QUERY_H

#include "../PoemStructures.h"
#include <string>
#include <vector>

struct LoadedHymnRecord {
    int index = 0;
    std::string sourcePath;
    std::string exportBaseName;
    const Hymn* hymn = nullptr;
};

std::string buildLoadedHymnSummary(const std::vector<LoadedHymnRecord>& hymns);
std::string buildInteractiveActionPrompt();
std::string runSearchQuery(const std::vector<LoadedHymnRecord>& hymns, const std::string& query);
std::string compareSelectedVerses(
    const std::vector<LoadedHymnRecord>& hymns,
    const std::string& references);
std::string compareSelectedHymns(
    const std::vector<LoadedHymnRecord>& hymns,
    const std::string& references);
std::string compareSearchQueryAcrossHymns(
    const std::vector<LoadedHymnRecord>& hymns,
    const std::string& query);

#endif
