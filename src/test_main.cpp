#include "Configuration.hpp"
#include <iostream>

int main() {
    Configuration config;
    std::string filename = "../conf/simple.conf";
    config.parseConfigFile(filename);
    config.printConfig();

	std::string server_name = "localhost";
	std::cout << config.getPortNumber(server_name) << std::endl;
	std::cout << config.getPortNumber("www.example.com") << std::endl;
	std::cout << config.getPortNumber("test") << std::endl;

    return 0;
}
