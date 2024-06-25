#pragma once

#include <iostream>
#include <map>
#include <sstream>

typedef unsigned char uchar;

class RequestData;
class Configuration;

class Request {
  public:
    virtual ~Request() = 0;
    static void parseMessage(std::string& requestMessage,
                             RequestData& requestData);
    static std::string parseBodyByContentLength(uchar* begin,
                                                std::string length);
    static std::string parseBodyByTransferEncoding(uchar* begin, uchar* end);
};
