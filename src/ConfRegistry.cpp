#include <filesystem>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "ConfRegistry.h"

#include "Configuration.h"
#include "Configure.h"
#include "j-utils-system.h"
#include "j-utils-fs.h"
#include "j-utils-string.h"

void ConfRegistry::create(
	const std::string& name,
	const std::string& declaration,
	const boost::json::object& configuration
) {
	if (ConfRegistry::exists(name)) {
		throw std::runtime_error("Configuration with name " + name + " already exists");
	}

	// write to file
	ConfRegistry::write(
		name,
		declaration,
		configuration
	);
}

boost::json::object ConfRegistry::get(const std::string& name) {
	if (!ConfRegistry::exists(name)) {
		throw std::runtime_error("Configuration with name " + name + " does not exist");
	}

	boost::json::object confObj = utils::json::read(ConfRegistry::confPath(name)).as_object();

	// make sure the object is valid
	if (!confObj.contains("declaration")) {
		throw std::runtime_error("Configuration object is missing the declaration");
	}
	if (!confObj.contains("configuration")) {
		throw std::runtime_error("Configuration object is missing the configuration");
	}

	return confObj;
}

void ConfRegistry::update(
	const std::string& name,
	const std::string& declaration,
	const boost::json::object& configuration
) {
	// write to file
	ConfRegistry::write(
		name,
		declaration,
		configuration
	);
}

void ConfRegistry::write(
	const std::string& name,
	const std::string& declaration,
	const boost::json::object& configuration
) {
	// make sure necessary config directories exist
	ConfRegistry::install();

	// store config object to .json file
	std::filesystem::path confFilePath = ConfRegistry::confPath(name);

	boost::json::object confObject;
	confObject["name"] = name;
	confObject["declaration"] = declaration;
	confObject["configuration"] = configuration;

	utils::json::write(
		confFilePath,
		confObject
	);
}

std::string ConfRegistry::getDeclaration(const std::string& name) {
	boost::json::object confObj = ConfRegistry::get(name);
	return confObj["declaration"].as_string().c_str();
}

boost::json::value ConfRegistry::getConfiguration(const std::string& name) {
	auto queryStart = name.find(".");
	if (queryStart != std::string::npos) {
		// name contains the query [confName].path.to.value
		return ConfRegistry::getConfiguration(
			name.substr(0, queryStart),
			name.substr(queryStart + 1, name.length() - queryStart - 1)
		);
	}
	boost::json::object confObj = ConfRegistry::get(name);
	return confObj["configuration"].as_object();
}

boost::json::value ConfRegistry::getConfiguration(const std::string& name, const std::string& query) {
	boost::json::object conf = ConfRegistry::getConfiguration(name).as_object();
	return ConfRegistry::query(conf, query, name);
}

boost::json::value ConfRegistry::query(
	boost::json::object& conf,
	const std::string& query,
	const std::string& name
) {

	boost::json::object& context = conf;

	std::vector<std::string> path = utils::string::split(query, ".");

	std::string pathCurrent = name;

	int offset = 0;
	for (std::string& key: path) {
		if (!context.contains(key)) {
			throw std::runtime_error(pathCurrent + " does not contain key " + key);
		}

		bool last = offset == path.size() - 1;

		auto value = context[key];

		if (last) {
			// found the queried value
			return value;
		}

		// switch context, making sure current value is an object
		if (!value.is_object()) {
			throw std::runtime_error(pathCurrent + "." + key + " is not an object");
		}

		pathCurrent += "." + key;

		context = value.as_object();

		offset++;
	}

	throw std::runtime_error("Couldn't get the value, unknown error");
}

std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> ConfRegistry::queryDeclaration(
	const std::string& name,
	const std::string& query
) {
	std::string declaration = ConfRegistry::getDeclaration(name);

	ConfDeclarationParser parser;
	parser.setDeclaration(declaration);
	parser.parse();

	if (query == "") {return parser.configRoot;}

	ConfigContainerFieldDeclaration& context = parser.configRoot;

	std::vector<std::string> path = utils::string::split(query, ".");
	int offset = 0;
	std::string pathCurrent = name;

	while (path.size() > offset) {
		std::string keyCurrent = path[offset];
		pathCurrent += "." + keyCurrent;

		bool found = false;

		for (auto& child: context.children) {
			if (std::holds_alternative<ConfigContainerFieldDeclaration>(child)) {
				ConfigContainerFieldDeclaration& container = std::get<ConfigContainerFieldDeclaration>(child);
				if (container.name == keyCurrent) {
					if (path.size() - 1 == offset) {
						// found the container declaration queried
						return container;
					}
	
					// set context to current container
					found = true;
					context = container;
					break;
				}
			} else {
				ConfigFieldDeclaration field = std::get<ConfigFieldDeclaration>(child);
				if (field.name == keyCurrent) {
					if (path.size() - 1 > offset) {
						// not the last key in path, but we are not within a container
						throw std::runtime_error(
							"Trying to access " + keyCurrent + " from " + pathCurrent + " which is not an object"
						);
					}
	
					// found the field
					return field;
				}
			}
	
		}

		offset++;

		if (found) {continue;}

		throw std::runtime_error("Could not find " + query + " in declaration");
	}

	throw std::runtime_error("Could not find " + query + " in declaration");
}

bool ConfRegistry::exists(const std::string& name) {
	return utils::fs::exists(ConfRegistry::confPath(name));
}

void ConfRegistry::install() {
	auto appPath = ConfRegistry::appPath();
	if (!utils::fs::exists(appPath)) {
		utils::fs::mkdir(appPath);
	}

	auto confPath = ConfRegistry::confPath();
	if (!utils::fs::exists(confPath)) {
		utils::fs::mkdir(confPath);
	}
}

std::filesystem::path ConfRegistry::appPath() {
	return utils::fs::extendPath<1>(
		utils::system::appDataDir(),
		{ ".confhub" }
	);
}

std::filesystem::path ConfRegistry::confPath() {
	return utils::fs::extendPath<2>(
		utils::system::appDataDir(),
		{ ".confhub", "config" }
	);
}

std::filesystem::path ConfRegistry::confPath(const std::string& name) {
	std::string confFileName = name + ".json";
	return utils::fs::extendPath<1>(
		ConfRegistry::confPath(),
		{ confFileName.c_str() }
	);
}

void ConfRegistry::deleteConfiguration(const std::string& name) {
	if (!ConfRegistry::exists(name)) {
		throw std::runtime_error("Configuration " + name + " does not exist");
	}
	// delete config file
	std::filesystem::path confPath = ConfRegistry::confPath(name);
	std::filesystem::remove(confPath);
}