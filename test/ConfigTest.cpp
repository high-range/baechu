#include <iostream>

#include "../src/Configuration.hpp"

int main(int argc, char** argv) {
    std::string filename = "./conf/simple.conf";

    // Get the single instance of Configuration and initialize it
    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    std::string path = "localhost:1000/";

    std::string root = config.getRootDirectory(path);
    std::cout << "using full path: " << root << std::endl;

    std::string server_name = "localhost";
    std::string port_number = "";
    std::string location = "";
    root = config.getRootDirectory(server_name, port_number, location);
    std::cout << "using parameters: " << root << std::endl;

    std::string client_max_body_size = config.getClientMaxBodySize(path);
    std::cout << client_max_body_size << std::endl;

    return 0;
}
