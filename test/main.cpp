#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "server/Connector.hpp"

int main(int argc, char* argv[]) {
    std::string filename = "./conf/default.conf";
    if (argc > 1) {
        filename = argv[1];
    }

    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    Connector Connector;
    Connector.addServer(8080);
    Connector.addServer(8081);
    Connector.addServer(8082);
    Connector.addServer(8083);

    Connector.start();

    return 0;
}
