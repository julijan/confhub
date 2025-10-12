#include "Configure.h"
#include "print-nice/PrintNice.h"

#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char** argv) {

	PrintNice print;

	if (argc > 1) {
		if (strcmp(argv[1], "configure") == 0) {
			// interactive configure
			if (argc < 3) {
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
			
		}
	} else {
		// command not provided
		// TODO: show help
		print.warning("Expected command");
	}
	
	return 0;
}