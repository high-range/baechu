#pragma once

#include <vector>

class Response {
  public:
    virtual ~Response() = 0;
    static std::string messageGenerate(
        std::vector<std::pair<int, std::string> > responseMessageSource);
};
