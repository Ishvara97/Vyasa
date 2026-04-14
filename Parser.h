#ifndef PARSER_H
#define PARSER_H

#include "PoemStructures.h"
#include <string>

Hymn parseHymn(const std::string& filename);
Word buildWordDEV(const std::string& raw);
void enrichVerse(Verse& v);
#endif