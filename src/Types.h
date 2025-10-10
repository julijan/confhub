#include <string>
#include <variant>
#include <vector>

enum class ConfigFieldType {
	Number,
	String,
	Boolean,
	Array,
	Container,
	Never,
};

struct ConfigFieldDeclaration {
	std::string name;
	ConfigFieldType type;
	ConfigFieldType childType; // used for array fields, Never for other types
};

struct ConfigContainerFieldDeclaration : ConfigFieldDeclaration {
	ConfigContainerFieldDeclaration* parent = nullptr;
	std::vector<std::variant<ConfigContainerFieldDeclaration, ConfigFieldDeclaration>> children;
};