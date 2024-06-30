#pragma once

#include <map>
#include <string>

typedef std::map<std::string, std::string> Header;
typedef const unsigned char c_uchar;
typedef std::string::iterator StrIter;

class RequestUtility {
  private:
    virtual ~RequestUtility() = 0;
    static bool isObsFold(StrIter begin);
    static bool isWS(StrIter begin);
    static bool isFieldVchar(StrIter begin);
    static bool isPchar(StrIter begin);
    static bool isHexDigit(StrIter begin);
    static bool isHttpVersion(StrIter begin);
    static bool isTchar(StrIter begin);
    static bool isCRLF(StrIter begin);
    static std::string th_strtrim(const std::string& src, c_uchar target);
    static bool isNum(const std::string& str);
    static long long strtonum(const std::string& str);
    static long long hexTonum(const std::string& str);

    friend class Request;
};

enum State {
    RequestLineStart,
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
