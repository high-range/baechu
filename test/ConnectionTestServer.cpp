#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "Manager.hpp"
#include "RequestData.hpp"
#include "server/Connector.hpp"

const int BUFFER_SIZE = 1024;

class Webserv : public Connector {
  public:
    Webserv(int port) : Connector(port) {}

  protected:
    void handleRequest(int client_fd) {
        std::string request;
        char buffer[BUFFER_SIZE];

        while (true) {
            int bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

            if (bytes_read < 0) {
                // Check if it's a non-blocking mode error (temporary condition)
                // or a genuine failure (permanent error)
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // std::cerr << "Data is not ready yet." << std::endl;
                    break;
                } else {
                    std::cerr
                        << "Failed to read from socket: " << strerror(errno)
                        << std::endl;
                    close(client_fd);
                    return;
                }
            } else if (bytes_read == 0) {
                std::cerr << "Connection closed by client." << std::endl;
                break;
            }

            request.append(buffer, bytes_read);
            std::cout << bytes_read << " bytes read. Request message appended"
                      << std::endl;
        }

        // prepare request, serverAddr, and clientAddr
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
    if (argc > 1) {
        filename = argv[1];
    }

    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    Webserv server(8080);
    server.start();

    return 0;
}
