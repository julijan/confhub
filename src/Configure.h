#pragma once
#include "ConfDeclarationParser.h"
#include "Configuration.h"
#include <string>

class Configure {
public:
	Configure(const std::string& confName, const std::string& declarationFilePath);

	// interactively configure using current declaration
	void interactive();

	// non-interactive configure
	void fromFile(const std::string& configFile);
	
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