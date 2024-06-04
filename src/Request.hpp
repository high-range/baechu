#pragma once

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

class RequestData;
class Configuration;

class Request {
  public:
    virtual ~Request() = 0;
    static void messageParse(std::string& requestMessage,
                             RequestData& requestData,
                             const Configuration& configuration);

    enum State {
        StartLineStart,
        Method,
        MethodEnd,
        RequestTargetStart,
        AbsolutePath,
        Query,
        RequestTargetEnd,
        HTTPVersion,
        StartLineEnd,
        HeaderStart,
        FieldName,
        WhiteSpace,
        FieldValue,
        FieldContent,
        ObsFold,
        HeaderEnd,
        Body
    };

    static bool isObsFold(const char* str);
    static bool isWS(const char c);
    static bool isFieldVchar(const char c);
    static bool isPchar(const char* str);
    static bool isHexDigit(const char c);
    static bool isHttpVersion(const char* str);
    static bool isTchar(const char c);
    static bool isCRLF(const char* str);
    static std::string th_substr(const char* src, const int start,
                                 const int end);
    static std::string th_strtrim(const std::string& src, const char target);
};
