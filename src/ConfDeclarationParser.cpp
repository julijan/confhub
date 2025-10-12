#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "ConfDeclarationParser.h"

ConfDeclarationParser::ConfDeclarationParser(const std::string& fileName) {
	// init config root
	this->configRoot.name = "root";
	this->configRoot.type = ConfigFieldType::Container;
	this->configRoot.childType = ConfigFieldType::Never;

	// set context to config root
	this->context = &this->configRoot;

	// load config from file
	this->load(fileName);

	// initialize tokenizer
	this->tokenizer = new ConfTokenizer(this->content);
}

ConfDeclarationParser::~ConfDeclarationParser() {
	delete this->tokenizer;
}

void ConfDeclarationParser::load(const std::string& fileName) {
	if (!std::filesystem::exists(fileName)) {
		throw std::runtime_error("File " + fileName + " does not exist");
	}

	std::fstream stream;
	stream.open(fileName);

	if (!stream.is_open()) {
		throw std::runtime_error("Error opening file for reading");
	}

	std::string buffer;
	while (std::getline(stream, buffer)) {
		this->content += buffer + "\n";
	}
	stream.close();
}

void ConfDeclarationParser::parse() {
	std::string name = "";
	std::string type = "";
	std::string elementType = "";
	while (this->tokenizer->peekNext().type != ConfTokenType::End) {
		ConfToken token = this->tokenizer->next();
		ConfToken tokenNext = this->tokenizer->peekNext();

		// must start with a symbol
		if (token.type != ConfTokenType::Symbol) {
			throw std::runtime_error("Expected symbol at " + std::to_string(token.start));
		}

		name = token.value;

		if (tokenNext.type != ConfTokenType::Colon) {
			throw std::runtime_error(
				"Expected colon at " +
				std::to_string(tokenNext.start) +
				", found " + this->tokenizer->typeName(tokenNext.type) + " with value " + tokenNext.value
			);
		}

		// consume colon
		this->tokenizer->next();

		// colon must be followed by type or BraceOpen
		token = this->tokenizer->next();

		if (token.type == ConfTokenType::BraceOpen) {
			// brace open, create a new container in current context and set it as a context
			ConfigContainerFieldDeclaration container;
			container.type = ConfigFieldType::Container;
			container.name = name;
			container.parent = this->context;

			// add new container to current context children
			this->context->children.push_back(container);

			// set new container as context
			this->context = (ConfigContainerFieldDeclaration*)&this->context->children.back();

			// current iteration is done
			continue;
		}

		
		// no context switch, expect a type name (symbol)

		if (token.type != ConfTokenType::Symbol) {
			throw std::runtime_error("Expected symbol at " + std::to_string(token.start));
		}

		type = token.value;

		if (type != "number" && type != "string" && type != "boolean" && type != "Array") {
			throw std::runtime_error(
				"Expected type name, found " + type + " at " + std::to_string(token.start)
			);
		}

		// if Array, expect element type definition
		if (type == "Array") {
			// expect <
			token = this->tokenizer->next();
			if (token.type != ConfTokenType::LessThan) {
				throw std::runtime_error("Expected < at " +  std::to_string(token.start));
			}

			// expect type (element type)
			token = this->tokenizer->next();
			if (token.type != ConfTokenType::Symbol) {
				throw std::runtime_error("Expected symbol at " + std::to_string(token.start));
			}

			if (token.value == "Array") {
				throw std::runtime_error(
					"Multi-dimensional arrays not supported at" + std::to_string(token.start)
				);
			}

			if (token.value != "number" && token.value != "string" && token.value != "boolean") {
				throw std::runtime_error(
					"Expected type name, found " + token.value + " at " + std::to_string(token.start)
				);
			}
			
			elementType = token.value;

			// expect >
			token = this->tokenizer->next();
			if (token.type != ConfTokenType::GreaterThan) {
				throw std::runtime_error("Expected > at " +  std::to_string(token.start));
			}
		}

		token = this->tokenizer->next();
		if (token.type != ConfTokenType::Semicolon) {
			throw std::runtime_error("Expexted semicolon at " + std::to_string(token.start));
		}

		ConfigFieldDeclaration field;
		field.name = name;
		field.type = this->stringToType(type);

		if (field.type == ConfigFieldType::Array) {
			field.childType = this->stringToType(elementType);
		}

		this->context->children.push_back(field);

		// if followed by closing brace switch context to parent of current context
		if (this->tokenizer->peekNext().type == ConfTokenType::BraceClose) {
			if (this->context->parent == nullptr) {
				throw std::runtime_error("Unexpexted closing brace at " + std::to_string(token.start));
			}

			this->context = this->context->parent;

			// consume closing brace
			this->tokenizer->next();

			// closing brace is optionally followed by semicolon, consume if it is
			if (this->tokenizer->peekNext().type == ConfTokenType::Semicolon) {
				this->tokenizer->next();
			}
		}
	}
}

ConfigFieldType ConfDeclarationParser::stringToType(const std::string& type) const {
	if (type == "number") {
		return ConfigFieldType::Number;
	}
	if (type == "string") {
		return ConfigFieldType::String;
	}
	if (type == "boolean") {
		return ConfigFieldType::Boolean;
	}
	if (type == "Array") {
		return ConfigFieldType::Array;
	}

	return ConfigFieldType::Never;
}

std::string ConfDeclarationParser::typeToString(ConfigFieldType type) const {
	switch (type) {
		case ConfigFieldType::Number: return "number";
		case ConfigFieldType::String: return "string";
		case ConfigFieldType::Boolean: return "boolean";
		case ConfigFieldType::Array: return "Array";
		case ConfigFieldType::Container: return "container";
		case ConfigFieldType::Never: return "never";
	}
	return "unknown type";
}