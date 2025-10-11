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
	ConfigContainerFieldDeclaration configRoot;

	ConfDeclarationParser(const std::string& fileName);
	~ConfDeclarationParser();

	void load(const std::string& fileName);

	void parse();
	
	ConfigFieldType stringToType(const std::string& type) const;
	std::string typeToString(ConfigFieldType type) const;

private:
	std::string content;
	ConfTokenizer* tokenizer;
	ConfigContainerFieldDeclaration* context;


};