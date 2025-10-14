#include "Configure.h"
#include "ConfParser.h"
#include "ConfRegistry.h"
#include "print-nice/PrintNice.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

Configure::Configure(
	const std::string& confName,
	const std::string& declarationFilePath
):
	parserDeclaration(declarationFilePath)
{
	this->name = confName;
	this->parserDeclaration.parse();
}

Configure::Configure(const std::string& confName) {
	this->name = confName;
	ConfRegistry registry;
	this->parserDeclaration.setDeclaration(registry.getDeclaration(confName));
	this->parserDeclaration.parse();
}

void Configure::interactive() {
	this->interactive(this->parserDeclaration.configRoot, this->name);
	// at this point, all configuration values are set
	// store to file using registry
	PrintNice print;
	try {
		ConfRegistry registry;
		registry.create(
			this->name,
			this->parserDeclaration.declaration(),
			this->conf.json()
		);

		print.success("Configuration saved");
	} catch (std::runtime_error e) {
		print.error(std::string("Error saving configuration: ") + e.what());
	}
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
			ConfigFieldDeclaration field = std::get<ConfigFieldDeclaration>(child);
			this->updateFieldInteractive(field, confPath);
		}
	}
}

void Configure::fromFile(const std::string& confFile) {
	boost::json::value conf = utils::json::read(confFile);
	if (!conf.is_object()) {
		throw std::runtime_error("Configuration is expected to be an object");
	}
	Configure::fromContainer(
		this->parserDeclaration.configRoot,
		conf.as_object(),
		this->name,
		false
	);

	// store to config file
	PrintNice print;
	try {
		ConfRegistry registry;
		registry.create(
			this->name,
			this->parserDeclaration.declaration(),
			this->conf.json()
		);

		print.success("Configuration saved");
	} catch (std::runtime_error e) {
		print.error(std::string("Error saving configuration: ") + e.what());
	}
}

void Configure::fromContainer(
	const ConfigContainerFieldDeclaration& declaration,
	boost::json::object& values,
	const std::string& confPath,
	bool promptMissing
) {

	for (auto& child: declaration.children) {
		if (std::holds_alternative<ConfigContainerFieldDeclaration>(child)) {
			// another container, recurse
			ConfigContainerFieldDeclaration containerNext = std::get<ConfigContainerFieldDeclaration>(child);

			if (!values.contains(containerNext.name)) {
				if (promptMissing) {
					values[containerNext.name] = {};
				} else {
					throw std::runtime_error("Missing key " + confPath + "." + containerNext.name);
				}
			}

			auto valuesNext = values[containerNext.name];

			if (!valuesNext.is_object()) {
				throw std::runtime_error( confPath + "." + containerNext.name + " is not an object");
			}
			
			// add container to JSON config
			this->conf.addContainer(containerNext.name);
			
			// resume from container
			std::string confPathNext = confPath + "." + containerNext.name;
			this->fromContainer(
				containerNext,
				valuesNext.as_object(),
				confPathNext,
				promptMissing
			);

			// when current container is configured, jump to previous context
			this->conf.contextPrev();
		} else {
			// field
			ConfigFieldDeclaration field = std::get<ConfigFieldDeclaration>(child);

			// make sure key exists in values
			if (!values.contains(field.name)) {

				if (promptMissing) {
					// value missing, prompt enabled, prompt for value
					this->updateFieldInteractive(
						field,
						confPath
					);
					continue;
				}

				throw std::runtime_error("Missing key " + confPath + "." + field.name);
			}

			auto value = values[field.name];

			// entered value is valid, add to config
			switch (field.type) {
				case ConfigFieldType::Number: {
					if (!value.is_number()) {
						throw std::runtime_error(confPath + " expected to be a number");
					}
					if (value.kind() == boost::json::kind::double_) {
						this->conf.addNumber(field.name, (float)value.as_double());
					} else {
						this->conf.addNumber(field.name, (int)value.as_int64());
					}
					break;
				}
					
				case ConfigFieldType::String: {
					if (!value.is_string()) {
						throw std::runtime_error(confPath + " expected to be a string");
					}
					this->conf.addString(field.name, value.as_string().c_str());
					break;
				}

				case ConfigFieldType::Boolean: {
					if (!value.is_bool()) {
						throw std::runtime_error(confPath + " expected to be a boolean");
					}
					this->conf.addBoolean(field.name, value.as_bool());
					break;
				}

				case ConfigFieldType::Array: {
					if (!value.is_array()) {
						throw std::runtime_error(confPath + " expected to be an array");
					}

					// value is an array, make sure elements are of expected type
					boost::json::array& arr = value.as_array();

					for (auto el: arr) {
						if (field.childType == ConfigFieldType::Number && !el.is_number()) {
							throw std::runtime_error(
								"Some elements of array at " + confPath + " are not a number"
							);
						}

						if (field.childType == ConfigFieldType::String && !el.is_string()) {
							throw std::runtime_error(
								"Some elements of array at " + confPath + " are not a string"
							);
						}

						if (field.childType == ConfigFieldType::Boolean && !el.is_bool()) {
							throw std::runtime_error(
								"Some elements of array at " + confPath + " are not a boolean"
							);
						}
					}

					// element types correct
					if (field.childType == ConfigFieldType::Number) {
						std::vector<float> numbers;
						for (auto el: arr) {
							if (el.kind() == boost::json::kind::double_) {
								numbers.push_back(el.as_double());
							} else {
								numbers.push_back(el.as_int64());
							}
						}
						this->conf.addNumberVector(field.name, numbers);
					} else if (field.childType == ConfigFieldType::String) {
						std::vector<std::string> strings;
						for (auto el: arr) {
							strings.push_back(el.as_string().c_str());
						}
						this->conf.addStringVector(field.name, strings);
					} else if (field.childType == ConfigFieldType::Boolean) {
						std::vector<bool> booleans;
						for (auto el: arr) {
							booleans.push_back(el.as_bool());
						}
						this->conf.addBooleanVector(field.name, booleans);
					}
				}

				default: { break; }
			}

		}
	}

}

void Configure::updateInteractive(const std::string& confPath, bool isEntry) {
	ConfRegistry registry;

	if (isEntry) {
		// load entire current configuration to this.conf
		this->conf.setJSON(registry.getConfiguration(this->name).as_object());
	}

	auto declarationForPath = registry.queryDeclaration(this->name, confPath);

	if (std::holds_alternative<ConfigFieldDeclaration>(declarationForPath)) {
		// updating a field
		this->updateFieldInteractive(
			std::get<ConfigFieldDeclaration>(declarationForPath), confPath
		);
	} else {
		// updating a container
		ConfigContainerFieldDeclaration container = std::get<ConfigContainerFieldDeclaration>(
			declarationForPath
		);
		for (auto& child: container.children) {
			std::string confPathNext = confPath == "" ? "" : confPath + ".";

			if (std::holds_alternative<ConfigFieldDeclaration>(child)) {
				confPathNext += std::get<ConfigFieldDeclaration>(child).name;
			} else {
				confPathNext += std::get<ConfigContainerFieldDeclaration>(child).name;
			}

			this->updateInteractive(
				confPathNext,
				false
			);
		}
	}

	if (isEntry) {
		registry.update(
			this->name,
			registry.getDeclaration(this->name),
			this->conf.json()
		);
	}
}

void Configure::updateFieldInteractive(const ConfigFieldDeclaration& field, const std::string& confPath) {
	PrintNice print;
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
			.text = this->parserDeclaration.typeToString(field.type).c_str(),
			.type = OutputType::Success,
			.style = TextStyle::Italic
		} <<

		// Array element type
		(
			field.type == ConfigFieldType::Array ?
			TextStyledToken{
				.text = ("<" + this->parserDeclaration.typeToString(field.childType) + ">").c_str(),
				.type = OutputType::Success,
				.style = TextStyle::Italic
			} :
			TextStyledToken{
				.text = "",
				.type = OutputType::Success,
				.style = TextStyle::Italic
			}
		) <<

		StreamOut();

		// clear previous value
		value = "";

		// read the user provided string into value
		std::getline(std::cin, value);

		if (field.type != ConfigFieldType::Array) {
			// simple values
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
						break;
					}

					default: { break; }
				}

				// value was valid and added to conf, nothing else to be done for current field
				break;
			}
		} else {
			// array
			try {
				ConfParser parser(value);
				if (field.childType == ConfigFieldType::Number) {
					std::vector<float> values = parser.parseNumberArray();
					this->conf.addNumberVector(field.name, values);
					break;
				} else if (field.childType == ConfigFieldType::String) {
					std::vector<std::string> values = parser.parseStringArray();
					this->conf.addStringVector(field.name, values);
					break;
				} else if (field.childType == ConfigFieldType::Boolean) {
					std::vector<bool> values = parser.parseBooleanArray();
					this->conf.addBooleanVector(field.name, values);
					break;
				}

				throw std::runtime_error(
					"Unhandled array element type " +
					this->parserDeclaration.typeToString(field.childType)
				);

			} catch(std::runtime_error e) {
				print.error(std::string("Array values error: ") + e.what());
			}
		}
		print.error("Invalid value, expected " + this->parserDeclaration.typeToString(field.type));
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

void Configure::updateDeclaration(const std::string& declarationPathNew) {

	if (!std::filesystem::exists(declarationPathNew)) {
		throw std::runtime_error("File " + declarationPathNew + " not found");
	}

	// load contents of the new declaration file
	std::string declarationNew;
	std::string buffer;
	std::fstream stream(declarationPathNew);
	if (!stream.is_open()) {
		throw std::runtime_error("Error opening " + declarationPathNew);
	}
	while (std::getline(stream, buffer)) {
		declarationNew += buffer + "\n";
	}
	stream.close();

	// parse old declaration
	std::string declarationOldString = ConfRegistry::getDeclaration(this->name);
	this->parserDeclaration.setDeclaration(declarationOldString);
	this->parserDeclaration.parse();
	ConfigContainerFieldDeclaration declarationOld = this->parserDeclaration.configRoot;

	// set old configuration
	boost::json::object config = ConfRegistry::getConfiguration(name).as_object();
	this->conf.setJSON(config);

	// set new declaration to parser
	this->parserDeclaration.setDeclaration(declarationNew);
	this->parserDeclaration.parse();

	// update configuration to store new declaration
	ConfRegistry registry;
	registry.update(this->name, declarationNew, config);

	this->fromContainer(
		this->parserDeclaration.configRoot,
		config,
		this->name,
		true
	);

	// update config file
	PrintNice print;
	try {
		ConfRegistry registry;
		registry.update(
			this->name,
			this->parserDeclaration.declaration(),
			this->conf.json()
		);

		print.success("Declaration updated");
	} catch (std::runtime_error e) {
		print.error(std::string("Error saving configuration: ") + e.what());
	}
}

std::string Configure::fieldName(std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> field) {
	if (this->isContainer(field)) {
		return std::get<ConfigContainerFieldDeclaration>(field).name;
	}

	return std::get<ConfigFieldDeclaration>(field).name;
}

ConfigFieldType Configure::fieldType(std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> field) {
	if (this->isContainer(field)) {
		return std::get<ConfigContainerFieldDeclaration>(field).type;
	}
	return std::get<ConfigFieldDeclaration>(field).type;
}

bool Configure::isContainer(std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> field) {
	return std::holds_alternative<ConfigContainerFieldDeclaration>(field);
}

bool Configure::hasField(const ConfigContainerFieldDeclaration& container, const std::string& fieldName) {
	auto posExisting = std::find_if(
		container.children.begin(),
		container.children.end(),
		[this, &fieldName](auto& child) {
			return this->fieldName(child) == fieldName;
		}
	);
	return posExisting != container.children.end();
}

std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> Configure::getField(
	const ConfigContainerFieldDeclaration& container,
	const std::string& fieldName
) {
	auto posExisting = std::find_if(
		container.children.begin(),
		container.children.end(),
		[this, &fieldName](auto& child) {
			return this->fieldName(child) == fieldName;
		}
	);
	if (posExisting == container.children.end()) {
		// not found
		throw std::runtime_error("getField: no such field " + fieldName);
	}

	if (this->isContainer(*posExisting)) {
		return std::get<ConfigContainerFieldDeclaration>(*posExisting);
	}

	return std::get<ConfigFieldDeclaration>(*posExisting);
}