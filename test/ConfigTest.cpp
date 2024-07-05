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
    std::string location = "/test/cgi-bin/a";
    std::cout << "root: "
              << config.getRootDirectory(ip, port, server_name, location)
              << std::endl;

    std::cout << "default port: " << config.getDefaultPort() << std::endl;

    std::vector<int> ports = config.getPortNumbers();
    std::cout << "ports: ";
    for (size_t i = 0; i < ports.size(); i++) {
        std::cout << ports[i] << ' ';
    }
    std::cout << std::endl;

    std::cout << "error_page: "
              << config.getErrorPageFromServer(ip, port, server_name, "401")
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

    std::vector<std::string> extensions =
        config.getCgiExtensions(ip, port, server_name);
    std::cout << "cgi extensions: ";
    for (size_t i = 0; i < extensions.size(); i++) {
        std::cout << extensions[i] << ' ';
    }
    std::cout << std::endl;

    std::cout << "cgi path: " << config.getCgiPath(ip, port, server_name, ".py")
              << std::endl;

    std::cout << "isRedirect: "
              << config.isLocationHaveRedirect(ip, port, server_name, location)
              << std::endl;
    std::vector<std::string> redirection =
        config.getRedirectionInfo(ip, port, server_name, location);
    if (redirection.size() != 0) {
        std::cout << "status code: " << redirection[0] << std::endl;
        std::cout << "path: " << redirection[1] << std::endl;
    }

    std::string port_number = "8080";
    std::cout << stringToInteger(port_number) << std::endl;
    return 0;
}