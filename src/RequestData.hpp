#pragma once

#include <map>
#include <utility>
#include <vector>

typedef std::map<std::string, std::string> Header;

class RequestData {
  private:
    struct StartLine {
        std::string method;
        std::string requestTarget;
        std::string path;
        std::string query;
        std::string version;
    } startLine;
    Header header;
    std::string body;
    // request message parsing data

    struct ClientData {
        std::string port;
        std::string ip;
    } clientData;
    // client information

    void setMethod(const std::string& method);
    void setRequestTarget(const std::string& requestTarget);
    void setPath(const std::string& path);
    void setQuery(const std::string& query);
    void setVersion(const std::string& version);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    const std::string getBodyHeaderName();
    // RequestData에 대한 setter 함수 정의

  public:
    void setPort(const std::string& port);
    void setIP(const std::string& ip);
    std::string getMethod() const;
    std::string getRequestTarget() const;
    std::string getPath() const;
    std::string getQuery() const;
    std::string getVersion() const;
    Header getHeader() const;
    std::string getBody() const;
    std::string getPort() const;
    std::string getIP() const;
    // RequestData에 대한 getter 함수 정의

    friend class Request;
};
