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
    ResponseData resolveErrorPage(ResponseData& response);

  private:
    const RequestData& request;

    std::string ip;
    std::string port;
    std::string serverName;
    std::string method;
    std::string path;
    std::string location;
    std::string fullPath;
    // used for dynamic request only
    std::string pathInfo;
    std::string scriptName;
    std::string exePath;  // ex) /usr/bin/python3

    void setPath(const std::string& path);
    std::string getFullPath(const std::string& path);

    bool isStatic;
    ResponseData handleStaticRequest();
    ResponseData handleDynamicRequest();
    // used in handleStaticRequest
    ResponseData doGetFile();
    ResponseData doGetDirectory();
    ResponseData doGet();
    ResponseData doPost();
    ResponseData doDelete();
    // used in handleDynamicRequest
    CgiEnvMap createCgiEnvMap();
    std::string runCgi();
};

bool isUtf8(const std::string& str);
std::string lower(std::string s);
std::string getServerName(std::string host);
char** makeArgs(const std::string& exePath, const std::string& scriptName);
char** makeEnvp(CgiEnvMap& envMap);
bool isExecutable(const std::string& path);
std::string loadErrorPage(int statusCode, const std::string errorPagePath);
