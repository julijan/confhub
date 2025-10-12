#include "Types.h"
#include "ConfTokenizer.h"
// ConfDeclarationParser parses configuration declaration files
// configuration declaration uses the same syntax as TypeScript

/*
Example configuration declaration file:
=======================================
numeric: number;
stringConf: string;
boolConf: boolean;
nested: {
	something: string;
	other: number;
}
arrayConf: Array<string>;
=======================================
*/

class ConfDeclarationParser {

public:
	// holds the parsed configuration
	ConfigContainerFieldDeclaration configRoot;

	ConfDeclarationParser(const std::string& fileName);
	~ConfDeclarationParser();

	// load declaration contents from given file to content
	void load(const std::string& fileName);

	// parse declaration contents, configuration is nested within configRoot
	void parse();
	
	// given a string return corresponding ConfigFieldType
	ConfigFieldType stringToType(const std::string& type) const;

	// given a ConfigFieldType return human-readable representation of it
	std::string typeToString(ConfigFieldType type) const;

	// gives access to raw declaration
	std::string declaration();

private:
	// raw declaration
	std::string content;
	
	ConfTokenizer* tokenizer;

	// current context
	ConfigContainerFieldDeclaration* context;


};