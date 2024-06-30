#include <iostream>

#include "../src/Configuration.hpp"

int main(int argc, char** argv) {
    std::string filename = "./conf/simple.conf";

    // Get the single instance of Configuration and initialize it
    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    std::string ip = "";
    std::string port = "";
    std::string server_name = "";
    std::string location = "/a/";
    std::cout << "root: "
              << config.getRootDirectory(ip, port, server_name, location)
              << std::endl;

    std::cout << "default port: " << config.getDefaultPort() << std::endl;

    std::vector<std::string> ports = config.getPortNumbers();
    std::cout << "ports: ";
    for (size_t i = 0; i < ports.size(); i++) {
        std::cout << ports[i] << ' ';
    }
    std::cout << std::endl;

    std::cout << "error_page: "
              << config.getErrorPageFromServer(ip, port, server_name, "400")
              << std::endl;

    std::cout << "autoindex: "
              << config.isDirectoryListingEnabled(ip, port, server_name,
                                                  location)
              << std::endl;

    std::vector<std::string> index_list =
        config.getIndexList(ip, port, server_name, location);
    std::cout << "index: ";
    for (size_t i = 0; i < index_list.size(); i++) {
        std::cout << index_list[i] << ' ';
    }
    std::cout << std::endl;

    std::cout << "cgi path: " << config.getCgiPath(ip, port, server_name, ".py")
              << std::endl;
    return 0;
}