#pragma once

#include <string>

class Configuration;
class RequestData;

class Manager {
  public:
    virtual ~Manager() = 0;
    static std::string run(std::string requestMessage, RequestData requestData);
};
