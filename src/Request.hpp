#pragma once

#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
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

  private:
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
        BodyStart,
        ContentLength,
        TransferEncoding,
        BodyEnd
    };

    enum ChunkState {
        Chunk,
        ChunkSize,
        ChunkExt,
        ChunkData,
        LastChunk,
        LastChunkExt,
        TrailerStart,
        TrailerFieldName,
        TrailerWhiteSpace,
        TrailerFieldValue,
        TrailerFieldContent,
        TrailerObsFold,
        TrailerEnd
    };

    static std::string contentLengthBodyParse(unsigned char* begin,
                                              std::string length);
    static std::string transferEncodingBodyParse(unsigned char* begin,
                                                 unsigned char* end);

    static bool isObsFold(const unsigned char* str);
    static bool isWS(const unsigned char c);
    static bool isFieldVchar(const unsigned char c);
    static bool isPchar(const unsigned char* str);
    static bool isHexDigit(const unsigned char c);
    static bool isHttpVersion(const unsigned char* str);
    static bool isTchar(const unsigned char c);
    static bool isCRLF(const unsigned char* str);
    static std::string th_substr(const unsigned char* src, const size_t start,
                                 const size_t end);
    static std::string th_strtrim(const std::string& src,
                                  const unsigned char target);
    static bool doesValidContentLength(const std::string& str);
    static bool doesExistContentLength(
        const std::map<std::string, std::string>& header);
    static bool doesExistTransferEncoding(
        const std::map<std::string, std::string>& header);
};
