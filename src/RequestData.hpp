#pragma once

#include <map>
#include <utility>
#include <vector>

class RequestData {
  private:
    struct StartLine {
        std::string method;
        std::string requestTarget;
        std::string path;
        std::string query;
        std::string version;
    } startLine;
    std::map<std::string, std::string> header;
    std::string body;

  public:
    std::string getMethod() const;
    std::string getRequestTarget() const;
    std::string getPath() const;
    std::string getQuery() const;
    std::string getVersion() const;
    std::map<std::string, std::string> getHeader() const;
    std::string getBody() const;

    friend class Request;
};
