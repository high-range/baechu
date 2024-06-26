#pragma once

#include <netinet/in.h>

#include <string>

class Configuration;

class Manager {
  public:
    virtual ~Manager() = 0;
    static std::string run(std::string requestMessage, sockaddr_in serverAddr,
                           sockaddr_in clientAddr);
};
