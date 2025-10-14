#pragma once

#include "Configure.h"
#include <filesystem>
#include <string>

#include "json/object.hpp"
#include <variant>

class ConfRegistry {
public:
	// store given declaration + configuration to a json file
	static void create(
		const std::string& name,
		const std::string& declaration,
		const boost::json::object& configuration
	);

	// return validated config object given configuration name
	static boost::json::object get(const std::string& name);

	// same as create, but it will overwrite existing config file
	static void update(
		const std::string& name,
		const std::string& declaration,
		const boost::json::object& configuration
	);

	// store config object to .json file
	static void write(
		const std::string& name,
		const std::string& declaration,
		const boost::json::object& configuration
	);

	// return declaration given configuration name
	static std::string getDeclaration(const std::string& name);

	// return entire configuration given configuration name
	static boost::json::value getConfiguration(const std::string& name);

	// return partial configuration given configuration name and query
	// query is a dot delimited path within the configuration
	static boost::json::value getConfiguration(const std::string& name, const std::string& query);

	// abstraction for querying the configuration object
	static boost::json::value query(
		boost::json::object& conf,
		const std::string& query,
		const std::string& name = "" // name only used to display in errors
	);

	// query declaration of given configuration
	// if query is an empty string returns the entire declaration object
	static std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> queryDeclaration(
		const std::string& name,
		const std::string& query
	);

	// list configurations
	static std::vector<std::string> list();

	static void deleteConfiguration(const std::string& name);

private:
	// check if configuration with given name exists
	static bool exists(const std::string& name);

	// create necessary directories (unless they already exist)
	static void install();

	// true if ~/.confhub/config exists
	static bool installed();

	// true if ~/.confhub exists
	static bool hasAppPath();

	// true if ~/.confhub/config exists
	static bool hasConfPath();

	// whatever ~/.confhub resolves to
	static std::filesystem::path appPath();

	// whatever ~/.confhub/config resolves to
	static std::filesystem::path confPath();
	static std::filesystem::path confPath(const std::string& name);
	
};