#include <filesystem>
#include <stdexcept>
#include <string>

#include "ConfRegistry.h"

#include "Configuration.h"
#include "j-utils-system.h"
#include "j-utils-fs.h"

void ConfRegistry::create(
	const std::string& name,
	const std::string& declaration,
	const boost::json::object& configuration
) {
	if (ConfRegistry::exists(name)) {
		throw std::runtime_error("Configuration with name " + name + " aleready exists");
	}

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

std::string ConfRegistry::getDeclaration(const std::string& name) {
	boost::json::object confObj = ConfRegistry::get(name);
	return confObj["declaration"].as_string().c_str();
}

boost::json::object ConfRegistry::getConfiguration(const std::string& name) {
	boost::json::object confObj = ConfRegistry::get(name);
	return confObj["configuration"].as_object();
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