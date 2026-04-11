#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <map>
#include "PoemStructures.h"
#include "json.hpp"
#include "analysis.h"

using json = nlohmann::json;

// Forward declarations
json letterToJson(const Letter& l);
json syllableToJson(const Syllable& s);
json wordToJson(const Word& w);
json verseToJson(const Verse& v);
json hymnToJson(const Hymn& h);



#endif