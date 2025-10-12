#pragma once

#include <string>
#include <vector>

#include "ConfTokenizer.h"

// ConfParser parses configuration instance or it's fragments

class ConfParser {

public:
	ConfParser(const std::string& conf);

	// array value parse, should be called when "[" is peeked
	std::vector<float> parseNumberArray();
	std::vector<std::string> parseStringArray();
	std::vector<bool> parseBooleanArray();

private:
	ConfTokenizer tokenizer;
};