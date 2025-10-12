#include "Configure.h"
#include "print-nice/PrintNice.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

Configure::Configure(
	const std::string& confName,
	const std::string& declarationFilePath
): parser(declarationFilePath) {
	this->name = confName;
	this->parser.parse();
}

void Configure::interactive() {
	this->interactive(this->parser.configRoot, this->name);
	// at this point, all configuration values are set
	std::cout << this->conf.string();
}

void Configure::interactive(const ConfigContainerFieldDeclaration& container, std::string& confPath) {
	for (auto& child: container.children) {
		if (std::holds_alternative<ConfigContainerFieldDeclaration>(child)) {
			// another container, recurse
			ConfigContainerFieldDeclaration containerNext = std::get<ConfigContainerFieldDeclaration>(child);
			
			// add container to JSON config
			this->conf.addContainer(containerNext.name);
			
			// resume from container
			std::string confPathNext = confPath + "." + containerNext.name;
			this->interactive(containerNext, confPathNext);

			// when current container is configured, jump to previous context
			this->conf.contextPrev();
		} else {
			// field, prompt for value
			PrintNice print;
			ConfigFieldDeclaration field = std::get<ConfigFieldDeclaration>(child);
			std::string value;
			while (true) {
				PrintStream stream;

				stream.separator = "";

				stream << "Enter value for " <<
				TextStyledToken{
					.text = confPath.c_str(),
					.type = OutputType::Info,
					.style = 0
				} <<
				"." <<
				TextStyledToken{
					.text = field.name.c_str(),
					.type = OutputType::Info,
					.style = TextStyle::Bold
				} <<
				": " <<
				TextStyledToken{
					.text = this->parser.typeToString(field.type).c_str(),
					.type = OutputType::Success,
					.style = TextStyle::Italic
				} <<
				StreamOut();

				std::cin >> value;
				if (this->valid(field, value)) {

					ConfTokenizer tokenizer(value);
					ConfToken token = tokenizer.next();

					// entered value is valid, add to config
					switch (field.type) {
						case ConfigFieldType::Number: {
							bool isFloat = value.find(".") != std::string::npos;
							if (isFloat) {
								this->conf.addNumber(field.name, std::stof(token.value));
							} else {
								this->conf.addNumber(field.name, std::stoi(token.value));
							}
							break;
						}
							
						case ConfigFieldType::String: {
							this->conf.addString(field.name, token.value);
							break;
						}

						case ConfigFieldType::Boolean: {
							this->conf.addBoolean(field.name, token.value == "true");
						}

						default: { break; }
					}

					// value was valid and added to conf, nothing else to be done for current field
					break;
				}
				print.error("Invalid value, expected " + this->parser.typeToString(field.type));
			}
		}
	}
}

bool Configure::valid(const ConfigFieldDeclaration& decl, const std::string& value) const {
	ConfTokenizer tokenizer(value);
	ConfToken token;
	try {
		token = tokenizer.next();
	} catch(std::runtime_error e) {
		PrintNice print;
		print.error(std::string("Syntax error: ") + e.what());
		return false;
	}
	if (decl.type == ConfigFieldType::Number) {
		return token.type == ConfTokenType::Number;
	}
	if (decl.type == ConfigFieldType::String) {
		return token.type == ConfTokenType::String;
	}
	if (decl.type == ConfigFieldType::Boolean) {
		return token.type == ConfTokenType::Symbol && token.value == "true" || token.value == "false";
	}

	return false;
}