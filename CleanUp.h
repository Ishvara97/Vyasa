//Cleanup
#ifndef CLEANUP_H
#define CLEANUP_H

#include <string>
#include<vector>

std::vector<std::string> split(const std::string& s, char delimiter);
std::vector<std::string> splitWords(const std::string& line);
std::vector<std::string> splitUTF8(const std::string& str);

//Swara Testing
bool isSwara(const std::string& ch);
bool isIgnorableSymbol(const std::string& ch);
#endif