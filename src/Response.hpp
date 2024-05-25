#pragma once

#include <iostream>

class Response {
  public:
    std::string setStatusCode();
    std::string setBody();

  private:
    std::string statusCode;
    std::string body;
};
