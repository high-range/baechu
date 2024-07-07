#pragma once

#include "ResponseData.hpp"

class Response {
  public:
    virtual ~Response() = 0;
    static std::string messageGenerate(ResponseData data);
};

std::string getReasonPhrase(int statusCode);