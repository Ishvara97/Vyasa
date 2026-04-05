//Cleanup
#ifndef CLEANUP_H
#define CLEANUP_H

#include <string>
#include<vector>

std::vector<std::string> splitLines(const std::string& text);

//For Grapheme/Unicode Normalization
std::vector<std::string> splitUTF8(const std::string& s);

//splitWords
std::vector<std::string> splitWords(const std::string& line);

#endif