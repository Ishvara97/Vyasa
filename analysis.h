#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "PoemStructures.h"
#include <map>
#include <string>


std::map<std::string, int> getLetterFrequency(const Verse& v);
std::map<std::string, int> getHymnLetterFrequency(const Hymn& h);
std::map<std::string, int> getSwaraFrequency(const Verse& v);
std::map<std::string, int> getHymnSwaraFrequency(const Hymn& h);
std::vector<std::string> getSyllablePattern(const Verse& v);
std::map<std::string, int> getNGrams(const Verse& v, int n);
std::map<std::string, int> getPhonemeClassFrequency(const Verse& v);
std::map<std::string, int> getHymnPhonemeClassFrequency(const Hymn& h);
std::vector<std::string> getConsonantClassSequence(const Verse& v);
std::vector<std::string> findWordsWithClass(const Verse& v, ConsonantClass cls);
void exportHymnAnalysisCSV(const Hymn& h, const std::string& filename);

#endif
