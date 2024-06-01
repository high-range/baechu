#pragma once

#include <iostream>
#include <vector>

class RequestData;
class Configuration;

class Request {
  public:
    virtual ~Request() = 0;
    static void MessageParse(std::string& requestMessage,
                             RequestData& requestData,
                             const Configuration& configuration);

  private:
    enum State {
        Method,
        Path,
        Version,
        FieldName,
        OWS,
        RWS,
        FirstFieldContent,
        SecondFieldContent,
        ObsFold,
        End
    } state;
    bool Request::isTChar(const char c);
    bool Request::isVChar(const char c);
    bool Request::isWS(const char c);
    bool Request::isCRLF(const char now, const char next);
};
