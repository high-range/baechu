#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "Manager.hpp"
#include "RequestData.hpp"
#include "server/Connector.hpp"

class Webserv : public Connector {
  public:
    Webserv(int port) : Connector(port) {}

  protected:
    void handleRequest(int client_fd) {
        char buffer[1024];
        int bytes_read = read(client_fd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            std::cerr << "Failed to read from socket." << std::endl;
            return;
        }

        // prepare request, serverAddr, and clientAddr
        std::string request(buffer, bytes_read);
        sockaddr_in serverAddr;
        socklen_t serverAddrLen = sizeof(serverAddr);
        getsockname(client_fd, (struct sockaddr*)&serverAddr, &serverAddrLen);
        sockaddr_in clientAddr = clientAddresses[client_fd];

        std::string response =
            Manager::run(request, RequestData(serverAddr, clientAddr));
        send(client_fd, response.c_str(), response.size(), 0);
        std::cout << "Response sent" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string filename = "./conf/default.conf";
    if (argc > 1) filename = argv[1];

    // Get the single instance of Configuration and initialize it
    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    Webserv server(8080);
    server.start();
    return 0;
}
