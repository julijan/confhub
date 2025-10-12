#pragma once

#include <cstddef>
#include <string>
#include <string_view>

// ConfTokenizer can tokenize configuration declaration and actual configuration

enum class ConfTokenType {
	Symbol,
	String,
	Number,
	BraceOpen,
	BraceClose,
	SquareBracketOpen,
	SquareBracketClose,
	LessThan,
	GreaterThan,
	Comma,
	Colon,
	Semicolon,
	End,
	Unknown
};

struct ConfToken {
	ConfTokenType type;
	std::string value;
	size_t start;
	size_t end;
};

class ConfTokenizer {

public:
	ConfTokenizer(const std::string& config);

	// allows changing the config after instance was created
	void setConfig(const std::string& config);

	// peek next token
	ConfToken peekNext();

	// get and consume the next token
	ConfToken next();

	// return human readable name of given token type
	std::string typeName(ConfTokenType type) const;
	
private:
	// char at current offset
	char current();

	// ignore spaces (unless within a string), newline and tab chars
	bool ignoredChar();

	// symbols must start with a letter
	bool symbolStartChar();

	// symbols can contain letters, underscore and number
	bool symbolChar();

	bool numericChar();

	bool isEnd();

	std::string_view config;
	size_t offset = 0;
};