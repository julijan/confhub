#include "ConfParser.h"

#include <stdexcept>
#include <string>
#include <vector>

ConfParser::ConfParser(const std::string& conf): tokenizer(conf) {}

std::vector<float> ConfParser::parseNumberArray() {
	ConfToken token = this->tokenizer.next();
	if (token.type != ConfTokenType::SquareBracketOpen) {
		throw std::runtime_error("Expected [ at " + std::to_string(token.start));
	}

	std::vector<float> values;

	while (true) {
		token = this->tokenizer.next();

		// expect a number
		if (token.type != ConfTokenType::Number) {
			throw std::runtime_error(
				"Expected number at " + std::to_string(token.start) +
				" found " + this->tokenizer.typeName(token.type)
			);
		}

		values.push_back(std::stof(token.value));

		// number can be followed by comma or closing square bracket
		token = this->tokenizer.next();
		
		bool arrayEnd = token.type == ConfTokenType::SquareBracketClose;

		if (!arrayEnd && token.type != ConfTokenType::Comma) {
			throw std::runtime_error("Expected comma or closing square bracket at " + std::to_string(token.start));
		}

		if (arrayEnd) {
			break;
		}
	}

	return values;
}

std::vector<std::string> ConfParser::parseStringArray() {
	ConfToken token = this->tokenizer.next();
	if (token.type != ConfTokenType::SquareBracketOpen) {
		throw std::runtime_error("Expected [ at " + std::to_string(token.start));
	}

	std::vector<std::string> values;

	while (true) {
		token = this->tokenizer.next();

		// expect a string
		if (token.type != ConfTokenType::String) {
			throw std::runtime_error("Expected string at " + std::to_string(token.start));
		}

		values.push_back(token.value);

		// string can be followed by comma or closing square bracket
		token = this->tokenizer.next();
		
		bool arrayEnd = token.type == ConfTokenType::SquareBracketClose;

		if (!arrayEnd && token.type != ConfTokenType::Comma) {
			throw std::runtime_error("Expected comma or closing square bracket at " + std::to_string(token.start));
		}

		if (arrayEnd) {
			break;
		}
	}

	return values;
}

std::vector<bool> ConfParser::parseBooleanArray() {
	ConfToken token = this->tokenizer.next();
	if (token.type != ConfTokenType::SquareBracketOpen) {
		throw std::runtime_error("Expected [ at " + std::to_string(token.start));
	}

	std::vector<bool> values;

	while (true) {
		token = this->tokenizer.next();

		// expect a symbol
		if (token.type != ConfTokenType::Symbol) {
			throw std::runtime_error("Expected symbol at " + std::to_string(token.start));
		}
		// ... that contains a value coercible to boolean
		if (token.value != "true" && token.value != "false") {
			throw std::runtime_error("Expected boolean at " + std::to_string(token.start));
		}

		values.push_back(token.value == "true");

		// bool can be followed by comma or closing square bracket
		token = this->tokenizer.next();
		
		bool arrayEnd = token.type == ConfTokenType::SquareBracketClose;

		if (!arrayEnd && token.type != ConfTokenType::Comma) {
			throw std::runtime_error("Expected comma or closing square bracket at " + std::to_string(token.start));
		}

		if (arrayEnd) {
			break;
		}
	}

	return values;
}