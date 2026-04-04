//CleanUp

#include "CleanUp.h"
#include <sstream>

// Split by char delimiters
std::vector<std::string> split(const std::string& s, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(s);
	std::string item;

	while (getline(ss, item, delimiter)) {
		tokens.push_back(item);
	}

	return tokens;
}

// Split words by spaces
std::vector<std::string> splitWords(const std::string& line) {
	return split(line, ' ');
}

//UTF-& Splitting for Devanagari
std::vector<std::string> splitUTF8(const std::string& str) {
	std::vector<std::string> result;

	for (size_t i = 0; i < str.size();) {
		unsigned char c = str[i];
		size_t len = 1;

		if ((c & 0x80) == 0) len = 1;
		else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;

        result.push_back(str.substr(i, len));
        i += len;
    }

    return result;
}

//Swara Detection for udātta / anudātta
bool isSwara(const std::string& ch) {
	return (ch == "॑" || ch == "॒");
}

//Ignorable Dandas
bool isIgnorableSymbol(const std::string& ch) {
    return (
        ch == "|" ||
        ch == "।" ||
        ch == "॥"
    );
}