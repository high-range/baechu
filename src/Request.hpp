#pragma once

#include <iostream>

class Request {
  public:
    Request(std::string request);
    std::string getMethod();
    std::string getUrl();
    std::string getHost();
    std::string getBody();

  private:
    std::string method;
    std::string url;
    std::string host;
    std::string body;
};
