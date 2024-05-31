#pragma once

#include <string>

class Request {
  public:
    Request(const std::string& message);
    std::string getMethod() const;
    std::string getUrl() const;
    std::string getHost() const;
    std::string getBody() const;

  private:
    std::string method;
    std::string url;
    std::string host;
    std::string body;
};
