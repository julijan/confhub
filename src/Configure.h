#pragma once
#include "ConfDeclarationParser.h"
#include <string>

class Configure {
public:
	Configure(const std::string& confName, const std::string& declarationFilePath);

	// interactively configure using current declaration
	void interactive();
	
	// true if given value can be coerced to field declaration type
	bool valid(const ConfigFieldDeclaration& decl, const std::string& value) const;
	
private:
	// interactively configure given container, called by interactive()
	void interactive(const ConfigContainerFieldDeclaration& container, std::string& confPath);

	ConfDeclarationParser parser;
	std::string name;
};