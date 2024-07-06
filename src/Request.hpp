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
    static bool parseMessage(RequestData& requestData);
    static void parseHeader(RequestData& requestData);
    static void parseBodyByContentLength(RequestData& requestData);
    static void parseBodyByTransferEncoding(RequestData& requestData);
};
