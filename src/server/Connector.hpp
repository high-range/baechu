#pragma once

#include <sys/event.h>

#include <string>
#include <vector>

class Connector {
  public:
    Connector(int port);
    void start();
    void stop();

  private:
    int port;
    int serverFd;
    int kq;
    std::vector<struct kevent> changes;

    void setupServer();
    void handleConnections();
    void setNonBlocking(int fd);
    void addEvent(int fd, int filter, int flags);

  protected:
    virtual void handleRequest(int client_fd) = 0;
};