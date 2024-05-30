#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>

#include "RequestData.hpp"

class Parser {
  public:
    static void startLineParse(std::string& requestMessage,
                               RequestData& requestData);
    static void headerParse(std::string& requestMessage,
                            RequestData& requestData);
    static void bodyParse(std::string& requestMessage,
                          RequestData& requestData);

  private:
    enum StartLineState { Method, Path, Version } startLineState;
    enum HeaderState {
        FieldName,
        OWS,
        RWS,
        FirstFieldContent,
        SecondFieldContent,
        ObsFold,
        End
    } headerState;

    static void startLineTokenize(std::string& requestMessage,
                                  RequestData& requestData);
    static void headerTokenize(std::string& requestMessage,
                               RequestData& requestData);
    static void startLineSyntaxCheck(const RequestData& requestData);
    static void startLineSemanticCheck(const RequestData& requestData);
    // static void headerSyntaxCheck(const RequestData& requestData);
    // static void headerSemanticCheck(const RequestData& requestData);
    static bool isTchar(const char c);
    static bool isVChar(const char c);
    static bool isWS(const char c);
    static bool isCRLF(const char now, const char next);
};

#endif
