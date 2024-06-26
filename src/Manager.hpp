#pragma once

#include <netinet/in.h>

#include <string>

class Configuration;
class RequestData;

class Manager {
  public:
    virtual ~Manager() = 0;
    static std::string run(std::string requestMessage, RequestData requestData);
};
