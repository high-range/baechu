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
#define MAX_EVENTS 1000

class Connector {
  public:
    Connector();
    ~Connector();

    void connectServerPorts();
    void start();

  private:
    int kq;
    std::vector<int> serverSockets;

    void handleEvent(struct kevent& event);
    void acceptConnection(struct kevent& event);
    void handleRead(struct kevent& event);
    void handleWrite(struct kevent& event);
    void handleTimer(struct kevent& event);

    bool setNonBlocking(int fd);

    bool addWriteEvent(int fd, void* udata);
    bool addReadEvent(int fd, void* udata);
    bool addTimerEvent(int fd, void* udata);
    void deleteReadEvent(int fd);
    void deleteTimerEvent(int fd);
};
