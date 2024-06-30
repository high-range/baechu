#pragma once

#include <string>

typedef std::string::iterator StrIter;

class RequestData;
class Configuration;

class Request {
  public:
    virtual ~Request() = 0;
    static void parseMessage(std::string& requestMessage,
                             RequestData& requestData);
    static std::string parseBodyByContentLength(std::string body,
                                                std::string length);
    static std::string parseBodyByTransferEncoding(StrIter begin);
};
