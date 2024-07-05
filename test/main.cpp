#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "server/Connector.hpp"

int main(int argc, char* argv[]) {
    std::string filename = "./conf/default.conf";
    if (argc == 2) {
        filename = argv[1];
    } else if (argc > 2) {
        std::cerr << "Usage: ./webserv <config_file>" << std::endl;
        return 1;
    }

    Configuration& config = Configuration::getInstance();
    try {
        config.initialize(filename);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    Connector Connector;
    std::vector<int> ports = config.getPortNumbers();
    for (size_t i = 0; i < ports.size(); i++) {
        Connector.addServer(ports[i]);
    }

    Connector.start();

    return 0;
}
