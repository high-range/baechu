#pragma once

#include <vector>

class Response {
  public:
    virtual ~Response() = 0;
    static std::string messageGenerate(
        std::pair<int, std::string> responseMessageSource);
};
