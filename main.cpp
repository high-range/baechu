#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "Connector.hpp"

int main(int argc, char* argv[]) {
    Configuration& config = Configuration::getInstance();
    std::string filename = "./conf/default.conf";
    Connector connector;

    try {
        if (argc > 2) {
            throw "Usage: ./webserv <config_file>";
        } else if (argc == 2) {
            filename = argv[1];
        }
        config.initialize(filename);
        connector.connectServerPorts();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const char* e) {
        std::cerr << e << std::endl;
        return 1;
    }

    connector.start();

    return 0;
}
