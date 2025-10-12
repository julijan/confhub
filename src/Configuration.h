#pragma once
#include <stack>
#include <string>

#include "j-utils-json.h"


class Configuration {

public:
	Configuration();

	// add number to current context
	void addNumber(const std::string& key, int value);
	// add float to current context
	void addNumber(const std::string& key, float value);

	// add string to current context
	void addString(const std::string& key, const std::string& value);	

	// add boolean to current context
	void addBoolean(const std::string& key, bool value);

	// add a new container to current context and switch context to new container
	void addContainer(const std::string& key);

	// switch to previous context
	void contextPrev();

	// return configuration as JSON string
	std::string string();

	// return configuration JSON
	boost::json::object json();

private:
	boost::json::object confRoot;

	boost::json::object* context;

	std::stack<boost::json::object*> contextStack;

};