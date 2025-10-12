#pragma once
#include "ConfDeclarationParser.h"
#include "Configuration.h"
#include <string>

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
	
private:
	// interactively configure given container, called by interactive()
	void interactive(const ConfigContainerFieldDeclaration& container, std::string& confPath);

	// non-interactive configure
	void fromContainer(
		const ConfigContainerFieldDeclaration& declaration,
		boost::json::object& values,
		const std::string& confPath
	);

	ConfDeclarationParser parserDeclaration;
	std::string name;
	Configuration conf;
};