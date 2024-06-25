#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "Manager.hpp"
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

        // Process the request if it's non-empty
        if (!request.empty()) {
            // TODO: Send "ip, port number" to Manager.
            // (Need to implement after Refactoring)
            // sockaddr_in clientAddr = clientAddresses[client_fd];
            // std::string ip_address = inet_ntoa(clientAddr.sin_addr);
            // int port = ntohs(clientAddr.sin_port);
            std::string response =
                Manager::run(request, Configuration::getInstance());
            if (send(client_fd, response.c_str(), response.size(), 0) < 0) {
                std::cerr << "Failed to send response: " << strerror(errno)
                          << std::endl;
            } else {
                std::cout << "Response sent" << std::endl;
            }
        } else {
            std::cerr << "Received empty request." << std::endl;
        }

        close(client_fd);
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
