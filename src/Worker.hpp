#pragma once

#include <string>

#include "RequestData.hpp"
#include "Response.hpp"

#define VERSION "HTTP/1.1"
#define SERVER_SOFTWARE "baechu/0.1"
#define GATEWAY_INTERFACE "CGI/1.1"
#define HOST_HEADER "host"
#define CONTENT_TYPE_HEADER "content-type"
#define CONTENT_LENGTH_HEADER "content-length"
#define GET "GET"
#define POST "POST"
#define PUT "PUT"
#define DELETE "DELETE"

typedef std::map<std::string, std::string> CgiEnvMap;

class Worker {
  public:
    Worker(const RequestData& request);

    ResponseData handleRequest();

  private:
    const RequestData& request;

    std::map<std::string, std::string> header;  // Response header

    std::string host;
    std::string domain;
    std::string ip;
    std::string port;

    std::string path;
    std::string location;
    std::string fullPath;

    bool isStatic;
    std::string pathInfo;
    std::string scriptName;

    ResponseData handleStaticRequest();

    ResponseData doGetFile();
    ResponseData doGetDirectory();
    ResponseData doGet();

    ResponseData doPost();

    ResponseData doPut();

    ResponseData doDelete();

    std::string getFullPath(const std::string& path);

    ResponseData handleDynamicRequest();
    CgiEnvMap createCgiEnvMap();
    std::string runCgi();
};
