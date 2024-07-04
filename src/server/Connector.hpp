#pragma once
#include <netinet/in.h>
#include <sys/event.h>

#include <map>
#include <string>
#include <vector>

#include "Manager.hpp"
#include "RequestData.hpp"

#define BUFFER_SIZE 1024
#define BACKLOG 128
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
    bool acceptConnection(struct kevent& event);
    void closeConnection(int client_fd);
    void handleRead(struct kevent& event);
    void handleWrite(struct kevent& event);
    void setNonBlocking(int fd);
};
