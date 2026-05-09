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
    SimilarityScore rareWeightedBigram;
    SimilarityScore rareWeightedTrigram;
    SimilarityScore bigramPosition;
    SimilarityScore trigramPosition;
    double bigramJaccard = 0.0;
    double trigramJaccard = 0.0;
    int devLevenshteinDistance = 0;
    double devLevenshteinSimilarity = 0.0;
    int iastLevenshteinDistance = 0;
    double iastLevenshteinSimilarity = 0.0;
    double leftBigramEntropy = 0.0;
    double rightBigramEntropy = 0.0;
    double leftTrigramEntropy = 0.0;
    double rightTrigramEntropy = 0.0;
};

std::map<std::string, int> getLetterFrequency(const Verse& v);
std::map<std::string, int> getHymnLetterFrequency(const Hymn& h);
std::map<std::string, int> getSwaraFrequency(const Verse& v);
std::map<std::string, int> getHymnSwaraFrequency(const Hymn& h);
std::vector<std::string> getSyllablePattern(const Verse& v);
std::map<std::string, int> getNGrams(const Verse& v, int n);
std::map<std::string, int> getPhoneticNGrams(const Verse& v, int n);
std::map<std::string, int> getHymnPhoneticNGrams(const Hymn& h, int n);
std::map<std::string, int> getPhoneticNGramPositionProfile(const Verse& v, int n);
std::map<std::string, int> getHymnPhoneticNGramPositionProfile(const Hymn& h, int n);
double getPhoneticNGramEntropy(const Verse& v, int n);
double getHymnPhoneticNGramEntropy(const Hymn& h, int n);
double computeNGramJaccardSimilarity(
    const std::map<std::string, int>& left,
    const std::map<std::string, int>& right);
SimilarityScore compareRareWeightedNGramProfiles(
    const std::map<std::string, int>& left,
    const std::map<std::string, int>& right);
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
