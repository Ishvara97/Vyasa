//Cleanup
#ifndef CLEANUP_H
#define CLEANUP_H

#include <string>
#include <vector>
#include <sstream>
#include "PoemStructures.h"

std::vector<std::string> splitLines(const std::string& text);

//For Grapheme/Unicode Normalization
std::vector<std::string> splitUTF8(const std::string& s);

//For IAST
std::vector<std::string> splitIAST(const std::string& s);

//splitWords
std::vector<std::string> splitWords(const std::string& line);

//Vowel Detection
bool isVowel(const std::string& ch);

//LongVowel Detection
bool isLongVowel(const std::string& ch);

//IAST Vowel Detection
bool isIASTVowel(const std::string& ch);

//DandaFilter
bool isIgnorableSymbol(const std::string& s);

//CleanWords
std::string cleanWord(const std::string& w);
//
std::string join(const std::vector<std::string>& vec, const std::string& delim);
std::string lettersToString(const std::vector<Letter>& letters);
//ExportCSV
void exportFullCSV(const Hymn& h, const std::string& filename);

//ViramaCheck

bool isVirama(const std::string& ch);

#endif