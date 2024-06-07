#pragma once

#include <map>
#include <sstream>
#include <string>

typedef std::map<std::string, std::string> Headers;

static std::pair<int, std::string> statusCodes[] = {
    std::make_pair(200, "OK"),
    std::make_pair(201, "Created"),
    std::make_pair(204, "No Content"),
    std::make_pair(400, "Bad Request"),
    std::make_pair(404, "Not Found"),
    std::make_pair(405, "Method Not Allowed"),
    std::make_pair(500, "Internal Server Error"),
    std::make_pair(501, "Not Implemented"),
};

static std::map<int, std::string> reasonPhrases(
    statusCodes, statusCodes + sizeof(statusCodes) / sizeof(statusCodes[0]));

class ResponseData {
  public:
    int statusCode;
    Headers headers;
    std::string body;

    ResponseData(int statusCode, Headers headers, std::string body);
    ResponseData(int statusCode);
    ResponseData(int statusCode, std::string body);
    ResponseData(int statusCode, Headers headers);
    ResponseData();
};
