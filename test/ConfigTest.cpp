#include <iostream>

#include "../src/Configuration.hpp"

int main(int argc, char** argv) {
    std::string filename = "./conf/simple.conf";

    // Get the single instance of Configuration and initialize it
    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    std::string port = "80";
    std::string serever_name = "localhost";
    std::string location = "/a/c";
    std::cout << "root: "
              << config.getRootDirectory(port, location, serever_name)
              << std::endl;

    std::vector<std::string> ports = config.getPortNumbers();
    std::cout << "ports: ";
    for (size_t i = 0; i < ports.size(); i++) {
        std::cout << ports[i] << ' ';
    }
    std::cout << std::endl;
    return 0;
}