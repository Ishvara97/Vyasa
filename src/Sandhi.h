#ifndef SANDHI_H
#define SANDHI_H

#include <string>
#include <vector>
#include "../PoemStructures.h"

std::string transliterateIASTToDEV(const std::string& text);
SandhiBoundary buildSandhiBoundary(
    const Word& surfaceWord,
    const std::vector<Word>& underlyingWords,
    size_t start,
    size_t leftIndex,
    size_t rightIndex);
std::vector<std::string> getNormalizedIASTWords(const Verse& v);

#endif
