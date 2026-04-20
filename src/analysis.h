#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "../PoemStructures.h"
#include "similarity.h"
#include <map>
#include <string>

struct VerseSimilarityComparison {
    int leftVerseNumber = 0;
    int rightVerseNumber = 0;
    SimilarityScore phonemeClass;
    SimilarityScore swara;
    SimilarityScore meterPattern;
};

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

std::vector<std::string> splitVerseIntoPadas(const std::string& text);
int countSyllablesInPada(const std::string& pada);
std::vector<int> getPadaSyllableCounts(const Verse& v);
std::string detectVerseMeter(const Verse& v);
std::string formatPadaCounts(const std::vector<int>& counts);
std::vector<VerseSimilarityComparison> getVerseSimilarityComparisons(const Hymn& h);

void exportHymnAnalysisCSV(const Hymn& h, const std::string& filename);
void exportVerseSimilarityCSV(const Hymn& h, const std::string& filename);

#endif
