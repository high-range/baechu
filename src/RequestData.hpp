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
    std::map<std::string, std::vector<std::string> > header;
    std::string body;

  public:
    std::string getMethod() const;
    std::string getPath() const;
    std::string getVersion() const;
    std::map<std::string, std::vector<std::string> > getHeader() const;
    std::string getBody() const;

    friend class Request;
};
