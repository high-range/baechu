#ifndef REQUESTMANAGER_HPP
#define REQUESTMANAGER_HPP

#include <iostream>
#include <vector>

#include "RequestData.hpp"

class RequestManager {
  public:
    virtual ~RequestManager() = 0;
    static std::vector<std::pair<int, std::string> > parse(
        std::string requestMessage, RequestData& requestData);
};

#endif
