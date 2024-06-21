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
#define DELETE "DELETE"

typedef std::map<std::string, std::string> CgiEnvMap;

class Worker {
  public:
    Worker(const RequestData& request);

    ResponseData handleRequest();

  private:
    const RequestData& request;

    std::map<std::string, std::string> header;  // Response header

    bool isStaticRequest(const RequestData& request);
    ResponseData handleStaticRequest(const RequestData& request);
    ResponseData doGet(const RequestData& request);
    ResponseData doPost(const RequestData& request);
    ResponseData doDelete(const RequestData& request);
    std::string getFullPath(const std::string& host, const std::string& path);
    ResponseData handleDynamicRequest();
    CgiEnvMap createCgiEnvMap();
    std::string runCgi();
};

// Utility functions used in Worker.cpp
bool isFile(const std::string& fullPath);
std::string doGetFile(const std::string& fullPath);
std::string getFileLink(const std::string& hostname, int port,
                        const std::string& dirname,
                        const std::string& filename);
std::string generateHTML(const std::string& fullPath, const std::string& host,
                         int port, const std::string& path);
std::string doGetDirectory(const std::string& fullPath, const std::string& host,
                           const std::string& path);
std::string generateFilename();
bool saveFile(const std::string& dir, const std::string& content);
