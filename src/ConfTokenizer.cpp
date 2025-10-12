#include "ConfTokenizer.h"
#include <stdexcept>

ConfTokenizer::ConfTokenizer(const std::string& config) {
	this->config = config;
}

ConfToken ConfTokenizer::peekNext() {
	size_t offsetStart = this->offset;

	ConfToken token;
	token.type = ConfTokenType::Unknown;
	token.start = this->offset;
	token.end = this->offset + 1;

	while (true) {
		if (this->ignoredChar()) {
			this->offset++;
			continue;
		}

		token.start = this->offset;
		token.end = this->offset + 1;

		if (this->isEnd()) {
			// last token
			token.type = ConfTokenType::End;
			break;
		}

		char current = this->current();

		if (current == '{') {
			token.type = ConfTokenType::BraceOpen;
			break;
		}

		if (current == '}') {
			token.type = ConfTokenType::BraceClose;
			break;
		}

		if (current == ':') {
			token.type = ConfTokenType::Colon;
			break;
		}

		if (current == ';') {
			token.type = ConfTokenType::Semicolon;
			break;
		}

		if (current == '<') {
			token.type = ConfTokenType::LessThan;
			break;
		}

		if (current == '>') {
			token.type = ConfTokenType::GreaterThan;
			break;
		}

		if (this->numericChar()) {
			// found begining of a number literal
			token.type = ConfTokenType::Number;
			token.value += current;
			this->offset++;
			bool decimalPointFound = false;
			while (this->numericChar() || (this->current() == '.' && !decimalPointFound) && !this->isEnd()) {
				token.value += this->current();
				token.end = this->offset;
				this->offset++;
			}
			break;
		}

		if (this->symbolStartChar()) {
			// found beginning of a symbol
			token.type = ConfTokenType::Symbol;
			token.value += this->current();
			this->offset++;
			// collect all characters of the current symbol
			while (this->symbolChar() && !this->isEnd()) {
				token.value += this->current();
				this->offset++;
				token.end = this->offset;
			}

			break;
		}

		if (current == '"') {
			// beginning of a string
			this->offset++;
			token.type = ConfTokenType::String;
			token.start = this->offset;

			while (this->current() != '"' && !this->isEnd()) {
				token.value += this->current();
				token.end = this->offset;
				this->offset++;
			}

			if (this->current() != '"') {
				// missing closing " for the string
				throw std::runtime_error("Missing closing \"");
			}

			// string complete
			break;
		}

	}

	// restore offset where it started
	this->offset = offsetStart;

	return token;
}

ConfToken ConfTokenizer::next() {
	ConfToken token = this->peekNext();
	this->offset = token.end;
	return token;
}

std::string ConfTokenizer::typeName(ConfTokenType type) const {
	switch (type) {
		case ConfTokenType::Number: return "number";
		case ConfTokenType::String: return "string";
		case ConfTokenType::Symbol: return "symbol";
		case ConfTokenType::BraceOpen: return "brace open";
		case ConfTokenType::BraceClose: return "brace close";
		case ConfTokenType::Colon: return "colon";
		case ConfTokenType::Semicolon: return "semicolon";
		case ConfTokenType::GreaterThan: return ">";
		case ConfTokenType::LessThan: return "<";
		case ConfTokenType::Unknown: return "unknown token";
		case ConfTokenType::End: return "end of file";
	}
	return "unhandled token";
}

char ConfTokenizer::current() {
	return this->config[this->offset];
}

bool ConfTokenizer::ignoredChar() {
	char current = this->current();
	return current == '\n' || current == ' ' || current == '\t';
}

bool ConfTokenizer::symbolStartChar() {
	char current = this->current();
	int offsetaz = current - 'a';
	if (offsetaz >= 0 && offsetaz <= 26) {
		return true;
	}

	int offsetAZ = current - 'A';
	if (offsetAZ >= 0 && offsetAZ <= 26) {
		return true;
	}

	return false;
}

bool ConfTokenizer::symbolChar() {
	char current = this->current();
	if (current == '_') {return true;}
	if (this->numericChar()) {return true;}
	return this->symbolStartChar();
}

bool ConfTokenizer::numericChar() {
	int offsetNumbers = this->current() - '0';
	if (offsetNumbers >= 0 && offsetNumbers <= 9) {
		return true;
	}
	return false;
}

bool ConfTokenizer::isEnd() {
	return this->offset >= this->config.size();
}