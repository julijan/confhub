#pragma once

#include <string>

class ConfLoader {
public:
	ConfLoader();

	// loads configuration from given file
	// if the file does not exist, throws an error
	// if the file has a .ts extension, it extracts the contents of outermost curly braces
	std::string load(const std::string& filePath);
};