#pragma once

#include <vector>

class Response {
  public:
    virtual ~Response() = 0;
    static std::string MessageGenerate(
        const std::vector<std::pair<int, std::string> > responseMessageSource);
};
