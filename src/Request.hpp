#pragma once

#include <map>
#include <string>

typedef std::string::iterator StrIter;
typedef std::map<std::string, std::string> Header;

class RequestData;
class Configuration;

class Request {
  public:
    virtual ~Request() = 0;
    static void parseMessage(std::string& requestMessage,
                             RequestData& requestData);
    static std::string parseBodyByContentLength(std::string body,
                                                std::string length);
    static std::string parseBodyByTransferEncoding(std::string body,
                                                   Header& bodyHeader);
};
