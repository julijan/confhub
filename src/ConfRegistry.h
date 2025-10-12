#pragma once

#include <filesystem>
#include <string>

#include "json/object.hpp"

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

	static void deleteConfiguration(const std::string& name);

private:
	// check if configuration with given name exists
	static bool exists(const std::string& name);

	// create necessary directories (unless they already exist)
	static void install();

	// whatever ~/.confhub resolves to
	static std::filesystem::path appPath();

	// whatever ~/.confhub/config resolves to
	static std::filesystem::path confPath();
	static std::filesystem::path confPath(const std::string& name);
	
};