#pragma once
#include "ConfDeclarationParser.h"
#include "Configuration.h"
#include <string>
#include <variant>

class Configure {
public:
	// this constructor is used when new configuration is being created
	// when updating existing configuration name-only overload variant should be used
	Configure(const std::string& confName, const std::string& declarationFilePath);

	// this constructor can only be used for editing existing configuration
	// it loads configuration's saved declaration
	Configure(const std::string& confName);

	// interactively configure using current declaration
	void interactive();

	// non-interactive configure
	void fromFile(const std::string& configFile);

	// interactively update configuration
	void updateInteractive(const std::string& confPath, bool isEntry);

	// interactively update value for given field
	void updateFieldInteractive(const ConfigFieldDeclaration& declaration, const std::string& confPath);
	
	// true if given value can be coerced to field declaration type
	bool valid(const ConfigFieldDeclaration& decl, const std::string& value) const;

	// update declaration of an existing configuration
	// interactively updating the config values to match the new declaration
	void updateDeclaration(const std::string& declarationPathNew);
	
private:
	// interactively configure given container, called by interactive()
	void interactive(const ConfigContainerFieldDeclaration& container, std::string& confPath);

	// non-interactive configure
	// if prompt missing is true user is prompted on missing values, otherwise error is thrown
	void fromContainer(
		const ConfigContainerFieldDeclaration& declaration,
		boost::json::object& values,
		const std::string& confPath,
		bool promptMissing
	);

	// return the field name of given field or container
	std::string fieldName(std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> field);

	// return the type of given field or container
	ConfigFieldType fieldType(std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> field);

	// true if given field is a container
	bool isContainer(std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> field);

	// check if given container has the given field
	bool hasField(const ConfigContainerFieldDeclaration& container, const std::string& fieldName);

	// get field from given container
	// should be checked if field exists using hasField first
	// if field does not exist this throws an error
	std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration> getField(
		const ConfigContainerFieldDeclaration& container,
		const std::string& fieldName
	);

	ConfDeclarationParser parserDeclaration;
	std::string name;
	Configuration conf;
};