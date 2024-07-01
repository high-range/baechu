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

    std::map<int, std::string> clientResponses;
    std::map<int, size_t> responseOffsets;

    void handleEvent(struct kevent& event);
    bool acceptConnection(int serverFd);
    void closeConnection(int client_fd);
    void handleRead(int client_fd);
    void handleWrite(int client_fd);
    void setNonBlocking(int fd);
};
