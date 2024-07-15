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

    bool isStatic;
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
    size_t determinePathEndPos(size_t dotPos);
    bool isCgiExtension(const std::string& ext);
    void configureCgiRequest(const std::string& ext, size_t pathEndPos);

    std::string runCgi();
    // used in runCgi
    bool setupPipes(int fds[2], int inFds[2], int errFds[2]);
    pid_t forkAndSetupChild(int fds[2], int inFds[2], int errFds[2]);
    void writeRequestBody(int inFd);
    void setupSignalHandler();
    std::string readFromChild(int fd);
    std::string handleTimeout(pid_t pid, int status);
};

bool isUtf8(const std::string& str);
std::string lower(std::string s);
std::string getServerName(std::string host);
char** makeArgs(const std::string& exePath, const std::string& scriptName, const std::string& pathInfo);
char** makeEnvp(CgiEnvMap& envMap);
bool isExecutable(const std::string& path);
std::string loadErrorPage(int statusCode, const std::string errorPagePath);
