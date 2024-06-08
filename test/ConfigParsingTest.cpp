#include <iostream>

#include "../src/Configuration.hpp"

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "Too many arguments" << std::endl;
        exit(EXIT_FAILURE);
    } else if (argc == 2) {
        std::string filename = argv[1];
        std::ifstream file(filename);
        Configuration config(filename);
        config.printConfig();

        std::string server_name = "www.example.com";
        std::string port_number = "8080";
        std::string location = "/";

        std::string value =
            config.getRootDirectory(server_name, port_number, location);
        if (value.empty()) {
            std::cout << "Not Found" << std::endl;
        } else {
            std::cout << "value: " << value << std::endl;
        }
    } else {
        Configuration config;
        config.printConfig();

        std::string server_name = "www.example.com";
        std::string port_number = "8080";
        std::string location = "/";

        std::string value =
            config.getRootDirectory(server_name, port_number, location);
        if (value.empty()) {
            std::cout << "Not Found" << std::endl;
        } else {
            std::cout << "value: " << value << std::endl;
        }
    }

    return 0;
}
