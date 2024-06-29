#pragma once
#include <netinet/in.h>
#include <sys/event.h>

#include <map>
#include <string>
#include <vector>

#include "Manager.hpp"
#include "RequestData.hpp"

#define BUFFER_SIZE 1024
#define MAX_EVENTS 10

class Connector {
  public:
    Connector();
    ~Connector();

    bool addServer(int port);
    void start();

  private:
    int kq;
    std::vector<int> serverSokets;
    std::map<int, sockaddr_in> clientAddresses;
    std::vector<struct kevent> changes;

    void handleEvent(struct kevent& event);
    bool acceptConnection(int serverFd);
    void setNonBlocking(int fd);
    void closeConnection(int client_fd);
    void handleRequest(int client_fd);
};
