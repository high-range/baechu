#pragma once

#include <string>

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
