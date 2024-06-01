#pragma once

#include <iostream>
#include <vector>

class RequestData;
class Configuration;

class Request {
  public:
    virtual ~Request() = 0;
    static std::vector<std::pair<int, std::string> > MessageParse(
        const std::string& requestMessage, RequestData& requestData,
        const Configuration& configuration);
};
