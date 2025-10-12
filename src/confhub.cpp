#include "ConfRegistry.h"
#include "Configure.h"
#include "print-nice/PrintNice.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {

	PrintNice print;

	if (argc > 1) {

		const char* command = argv[1];
		int commandArgCount = argc - 2;

		if (strcmp(command, "configure") == 0) {
			// interactive configure
			if (commandArgCount == 0) {
				// must provide a path to declaration
				print.error("Expected path to configuration declaration file");
				return 1;
			}

			// prompt config name
			std::string configName;
			while (configName.size() == 0) {
				print.info("Configuration name:");
				std::cin >> configName;
			}

			// clear cin buffer
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			Configure conf(configName, argv[2]);
			conf.interactive();
			
			return 0;
		}

		if (strcmp(command, "declaration") == 0) {
			// show declaration
			if (commandArgCount == 0) {
				print.error("Expected configuration name");
				return 1;
			}

			try {
				ConfRegistry registry;
				print.print(registry.getDeclaration(argv[2]).c_str());
				return 0;
			} catch(std::runtime_error e) {
				print.error(std::string("Error: ") + e.what());
				return 1;
			}
		}

		if (strcmp(command, "query") == 0) {
			// query configuration
			if (commandArgCount == 0) {
				print.error("Expected configuration name");
				return 1;
			}

			try {
				std::stringstream conf;
				ConfRegistry registry;
				if (commandArgCount == 1) {
					conf << registry.getConfiguration(argv[2]);
				} else {
					// query provided, narrow to queried
					conf << registry.getConfiguration(argv[2], argv[3]);
				}
				print.print(conf.str().c_str());
				return 0;
			} catch(std::runtime_error e) {
				print.error(std::string("Error: ") + e.what());
				return 1;
			}
		}

	} else {
		// command not provided
		// TODO: show help
		print.warning("Expected command");
	}
	
	return 0;
}