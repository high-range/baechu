#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "Manager.hpp"
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

        std::string request(buffer, bytes_read);
        std::string response = Manager::run(request, Configuration());
        send(client_fd, response.c_str(), response.size(), 0);
        std::cout << "Response sent" << std::endl;
    }
};

int main() {
    Webserv server(8080);
    server.start();
    return 0;
}
