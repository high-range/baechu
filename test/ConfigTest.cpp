#include <iostream>

#include "../src/Configuration.hpp"

int main(int argc, char** argv) {
    std::string filename = "./conf/simple.conf";

    // Get the single instance of Configuration and initialize it
    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    std::string host = "";
    std::string port = "8080";
    std::string location = "";
    std::cout << "3parameters root: "
              << config.getRootDirectory(host, port, location) << std::endl;

    std::string path = "localhost:8080/";
    std::string root = config.getRootDirectory(path);
    std::cout << "fullPath root: " << root << std::endl;

    std::cout << "default port: " << config.getDefaultPort() << std::endl;

    std::cout << "client max body size: " << config.getClientMaxBodySize(path)
              << std::endl;

    std::vector<std::string> cgi_extensions = config.getCgiExtensions(path);
    std::cout << "cgi: ";
    for (size_t i = 0; i < cgi_extensions.size(); i++) {
        std::cout << cgi_extensions[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "methods: " << config.getAllowedMethods(path) << ' '
              << config.isMethodAllowedFor(path, "GET") << std::endl;

    std::cout << "autoindex: " << config.isDirectoryListingEnabled(path)
              << std::endl;

    std::cout << "error_page: " << config.getErrorPageFromServer(path)
              << std::endl;

    return 0;
}