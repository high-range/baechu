#pragma once

#include <iostream>
#include <vector>

class RequestData;
class Configuration;

class Request {
  public:
    virtual ~Request() = 0;
    static void messageParse(std::string& requestMessage,
                             RequestData& requestData,
                             const Configuration& configuration);
};
