#pragma once

#include <netinet/in.h>

#include <string>

class Configuration;
class RequestData;
class ResponseData;

class Manager {
  public:
    virtual ~Manager() = 0;
    static std::string run(RequestData requestData);
    static std::string run(RequestData requestData, ResponseData responseData);
};
