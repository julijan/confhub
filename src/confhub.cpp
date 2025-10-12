#include "ConfRegistry.h"
#include "Configure.h"
#include "print-nice/PrintNice.h"

#include <cstring>
#include <fstream>
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

			if (commandArgCount == 1) {
				// interactive configure
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

			if (commandArgCount == 3) {
				// non-interactive configure
				// confhub configure [confName] /path/to/declaration.conf /path/to/configuration.json

				const char* name = argv[2];
				const char* pathDeclaration = argv[3];
				const char* pathConfiguration = argv[4];

				try {
					Configure conf(name, pathDeclaration);
					conf.fromFile(pathConfiguration);
					return 0;
				} catch (std::runtime_error e) {
					print.error(std::string("Error: ") + e.what());
					return 1;
				}
			}
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

		if (strcmp(command, "update") == 0) {
			// delete configuration
			if (commandArgCount == 0) {
				print.error("Expected configuration name");
				return 1;
			}

			try {
				std::string updatePath = argv[2];
				auto nameIndex = updatePath.find(".");
				if (nameIndex == std::string::npos) {
					// updating entire configuration
					Configure conf(updatePath);
					conf.updateInteractive("", true);
				} else {
					// updating a specific path
					Configure conf(updatePath.substr(0, nameIndex));
					conf.updateInteractive(updatePath.substr(nameIndex + 1), true);
				}
				print.success("Configuration updated");
				return 0;
			} catch(std::runtime_error e) {
				print.error(std::string("Error: ") + e.what());
				return 1;
			}
		}

		if (strcmp(command, "delete") == 0) {
			// delete configuration
			if (commandArgCount == 0) {
				print.error("Expected configuration name");
				return 1;
			}

			try {
				ConfRegistry registry;
				registry.deleteConfiguration(argv[2]);
				print.success("Configuration deleted");
				return 0;
			} catch(std::runtime_error e) {
				print.error(std::string("Error: ") + e.what());
				return 1;
			}
		}

		if (strcmp(command, "export") == 0) {
			// export configuration
			if (commandArgCount == 0) {
				print.error("Expected configuration name");
				return 1;
			}

			if (commandArgCount == 1) {
				print.error("Expected path to output file");
				return 1;
			}

			try {
				ConfRegistry registry;
				auto config = registry.getConfiguration(argv[2]);
				std::ofstream streamOut;
				streamOut.open(argv[3]);

				if (!streamOut.is_open()) {
					throw std::runtime_error(std::string("Error opening ") + argv[3] + " for writing");
					return 1;
				}

				streamOut << config;
				streamOut.flush();
				streamOut.close();

				print.success("Configuration exported");
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