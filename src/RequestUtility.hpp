#pragma once

#include <iostream>
#include <map>

typedef std::map<std::string, std::string> Header;
typedef const unsigned char c_uchar;

class RequestUtility {
  public:
    virtual ~RequestUtility() = 0;
    static bool isObsFold(c_uchar* str);
    static bool isWS(c_uchar c);
    static bool isFieldVchar(c_uchar c);
    static bool isPchar(c_uchar* str);
    static bool isHexDigit(c_uchar c);
    static bool isHttpVersion(c_uchar* str);
    static bool isTchar(c_uchar c);
    static bool isCRLF(c_uchar* str);
    static std::string th_substr(c_uchar* src, const size_t start,
                                 const size_t end);
    static std::string th_strtrim(const std::string& src, c_uchar target);
    static bool doesValidContentLength(const std::string& str);
    static bool doesExistContentLength(const Header& header);
    static bool doesExistTransferEncoding(const Header& header);
};

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
