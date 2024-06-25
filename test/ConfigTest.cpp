#include <iostream>

#include "../src/Configuration.hpp"

int main(int argc, char** argv) {
    std::string filename = "./conf/simple.conf";

    // Get the single instance of Configuration and initialize it
    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    std::string path = "localhost:8080/a/b/c";

    std::string root = config.getRootDirectory(path);
    std::cout << "using full path: " << root << std::endl;

    std::string server_name = "localhost";
    std::string port_number = "8080";
    std::string location = "/a/b/c";
    root = config.getRootDirectory(server_name, port_number, location);
    std::cout << "using parameters: " << root << std::endl;

    std::string client_max_body_size = config.getClientMaxBodySize(path);
    std::cout << client_max_body_size << std::endl;

    return 0;
}