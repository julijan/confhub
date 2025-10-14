#include "ConfLoader.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "ConfTokenizer.h"
#include "j-utils-string.h"
#include "PrintNice.h"

ConfLoader::ConfLoader() {}

std::string ConfLoader::load(const std::string& filePath) {

	std::string content;

	if (!std::filesystem::exists(filePath)) {
		throw std::runtime_error("File " + filePath + " does not exist");
	}

	std::fstream stream;
	stream.open(filePath);

	if (!stream.is_open()) {
		throw std::runtime_error("Error opening file for reading");
	}

	std::string buffer;
	while (std::getline(stream, buffer)) {
		content += buffer + "\n";
	}
	stream.close();

	if (utils::string::endsWith(filePath, ".ts")) {
		// TypeScript file used as declaration
		// it's assumed that only one type will be defined in this file
		// extract the code within the outer curly braces

		PrintNice print;
		print.info(
			"Using a TypeScript file as declaration. File is assumed to contain exactly one type definition."
		);

		ConfTokenizer tokenizer(content);
		size_t start = 0;
		size_t end = 0;
		while (true) {
			ConfToken token = tokenizer.next();
			if (start == 0 && token.type == ConfTokenType::BraceOpen) {
				// found first opening curly brace
				start = token.end;
				continue;
			}

			if (start > 0 && token.type == ConfTokenType::BraceClose) {
				end = token.start;
			}

			if (token.type == ConfTokenType::End) {
				break;
			}
		}

		if (end == 0 || start == end) {
			throw std::runtime_error("Error extracting the configuration from a TypeScript file");
		}

		// extract the relevant substring and set it as tokenizer config
		content = content.substr(start, end - start);
	}

	return content;
}
