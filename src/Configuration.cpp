#include "Configuration.h"
#include <sstream>
#include <string>
#include <vector>

Configuration::Configuration() {
	// initialize with context being the root object
	this->context = &this->confRoot;

	this->contextStack.push(this->context);
}

void Configuration::addNumber(const std::string& key, int value) {
	(*this->context)[key] = value;
}

void Configuration::addNumber(const std::string& key, float value) {
	(*this->context)[key] = value;
}

void Configuration::addString(const std::string& key, const std::string& value) {
	(*this->context)[key] = value;
}

void Configuration::addBoolean(const std::string& key, bool value) {
	(*this->context)[key] = value;
}

void Configuration::addNumberVector(const std::string& key, std::vector<float> values) {
	boost::json::array arr = boost::json::array();
	for (float val: values) {
		arr.push_back(val);
	}
	(*this->context)[key] = arr;
}

void Configuration::addStringVector(const std::string& key, std::vector<std::string> values) {
	boost::json::array arr = boost::json::array();
	for (const std::string& val: values) {
		arr.push_back(val.c_str());
	}
	(*this->context)[key] = arr;
}

void Configuration::addBooleanVector(const std::string& key, std::vector<bool> values) {
	boost::json::array arr = boost::json::array();
	for (bool val: values) {
		arr.push_back(val);
	}
	(*this->context)[key] = arr;
}

void Configuration::addContainer(const std::string& key) {
	// add new container to current context
	(*this->context)[key] = boost::json::object();

	boost::json::object* ctxNew = (boost::json::object*)(&((*this->context)[key]));
	
	// switch context to new container
	this->context = ctxNew;

	// add new context to contextStack
	this->contextStack.push(ctxNew);
}

void Configuration::contextPrev() {
	if (this->contextStack.size() > 1) {
		this->contextStack.pop();
	}
	this->context = this->contextStack.top();
}

std::string Configuration::string() {
	std::stringstream str;
	str << this->confRoot;
	return str.str();
}

boost::json::object Configuration::json() {
	return this->confRoot;
}