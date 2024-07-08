#pragma once

#include <map>
#include <sstream>
#include <string>

typedef std::map<std::string, std::string> Headers;

class ResponseData {
  public:
    int statusCode;
    std::string reasonPharse;
    Headers headers;
    std::string body;

    ResponseData(int statusCode, Headers headers, std::string body);
    ResponseData(int statusCode, Headers headers);
    ResponseData(int statusCode, std::string body);
    ResponseData(int statusCode);
    ResponseData withReasonPhrase(std::string reasonPhrase);
};
