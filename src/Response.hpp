#pragma once

#include <string>

class Response {
  public:
    Response(int code, const std::string& content);
    int getCode() const;
    std::string getContent() const;
    std::string toString() const;

  private:
    int code;
    std::string content;
};
