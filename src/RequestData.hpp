#pragma once

#include <map>
#include <utility>
#include <vector>

class RequestData {
  private:
    struct StartLine {
        std::string method;
        std::string path;
        std::string version;
    } startLine;
    std::map<std::string, std::string> header;
    std::string body;
    std::string query;

  public:
    std::string getMethod() const;
    std::string getPath() const;
    std::string getVersion() const;
    std::map<std::string, std::string> getHeader() const;
    std::string getBody() const;
    std::string getQuery() const;

    friend class Request;
};
