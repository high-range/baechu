#include "Request.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "RequestData.hpp"
#include "RequestUtility.hpp"
#include "ResponseData.hpp"

bool Request::parseMessage(RequestData& requestData) {
    ParseState state;
    std::string bodyHeaderName, bodyHeaderValue;

    while (true) {
        state = requestData.state;
        switch (state) {
            case Start:
                if (RequestUtility::isHeaderComplete(requestData.rawData)) {
                    parseHeader(requestData);
                    requestData.clearHeaderData();
                } else
                    return false;
            case HeaderIsParsed:
                bodyHeaderName = requestData.getBodyHeaderName();
                if (bodyHeaderName == "content-length" &&
                    RequestUtility::isNum(requestData.header[bodyHeaderName])) {
                    long long contentLength = RequestUtility::strtonum(
                        requestData.header[bodyHeaderName]);
                    long long maxBodySize = requestData.getClientMaxBodySize();
                    if (contentLength > maxBodySize) {
                        throw ResponseData(413);
                    }
                    requestData.state = Body_ContentLength;
                } else if (bodyHeaderName == "transfer-encoding" &&
                           requestData.header[bodyHeaderName] == "chunked") {
                    requestData.state = Body_TransferEncoding;
                } else if (bodyHeaderName == "") {
                    return true;
                } else
                    throw ResponseData(400);
                break;
            case Body_ContentLength:
                if (requestData.isBodyCompleteByContentLength()) {
                    parseBodyByContentLength(requestData);
                    return true;
                }
                return false;
            case Body_TransferEncoding:
                if (requestData.isBodyCompleteByTransferEncoding()) {
                    long long maxBodySize = requestData.getClientMaxBodySize();
                    if (static_cast<long long>(requestData.rawData.size()) >
                        maxBodySize) {
                        throw ResponseData(413);
                    }
                    parseBodyByTransferEncoding(requestData);
                    return true;
                }
                return false;
        }
    }
}

void Request::parseHeader(RequestData& requestData) {
    State state;
    size_t queryStart;
    std::string token;
    std::string fieldname, fieldvalue;
    StrIter begin;

    state = RequestLineStart;
    begin = requestData.rawData.begin();
    while (true) {
        switch (state) {
            case RequestLineStart:
                if (RequestUtility::isTchar(begin)) {
                    state = Method;
                } else
                    throw ResponseData(400);
                break;
            case Method:
                if (RequestUtility::isTchar(begin)) {
                    token += *begin;
                } else if (*begin == ' ') {
                    state = MethodEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case MethodEnd:
                requestData.setMethod(token);
                token.clear();
                state = RequestTargetStart;
            case RequestTargetStart:
                if (*begin == '/') {
                    state = AbsolutePath;
                } else
                    throw ResponseData(400);
                break;
            case AbsolutePath:
                if (RequestUtility::isPchar(begin) || *begin == '/') {
                    token += *begin;
                    if (*begin == '%') {
                        token += *(++begin);
                        token += *(++begin);
                    }
                } else if (*begin == '?') {
                    requestData.setPath(token);
                    token += *begin;
                    queryStart = token.size();
                    state = Query;
                } else if (*begin == ' ') {
                    requestData.setPath(token);
                    state = RequestTargetEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case Query:
                if (RequestUtility::isPchar(begin) || *begin == '/' ||
                    *begin == '?') {
                    token += *begin;
                    if (*begin == '%') {
                        token += std::string(begin + 1, begin + 3);
                        begin += 2;
                    }
                } else if (*begin == ' ') {
                    requestData.setQuery(token.substr(queryStart));
                    state = RequestTargetEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case RequestTargetEnd:
                requestData.setRequestTarget(token);
                token.clear();
                state = HTTPVersion;
                break;
            case HTTPVersion:
                if (RequestUtility::isHttpVersion(begin)) {
                    token = std::string(begin, begin + 8);
                    requestData.setVersion(token);
                    token.clear();
                    begin += 8;
                    state = StartLineEnd;
                } else
                    throw ResponseData(400);
                break;
            case StartLineEnd:
                if (RequestUtility::isCRLF(begin)) {
                    state = HeaderStart;
                    begin += 2;
                } else
                    throw ResponseData(400);
                break;
            case HeaderStart:
                if (RequestUtility::isTchar(begin)) {
                    state = FieldName;
                } else if (RequestUtility::isCRLF(begin))
                    state = HeaderEnd;
                else
                    throw ResponseData(400);
                break;
            case FieldName:
                if (RequestUtility::isTchar(begin)) {
                    fieldname += std::tolower(*begin);
                } else if (*begin == ':') {
                    state = WhiteSpace;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case WhiteSpace:
                if (RequestUtility::isWS(begin)) {
                    fieldvalue += *begin;
                    begin++;
                } else
                    state = FieldValue;
                break;
            case FieldValue:
                if (RequestUtility::isFieldVchar(begin)) {
                    state = FieldContent;
                } else if (RequestUtility::isObsFold(begin)) {
                    state = ObsFold;
                } else if (RequestUtility::isCRLF(begin)) {
                    fieldvalue = RequestUtility::th_strtrim(fieldvalue, ' ');
                    requestData.setHeader(fieldname, fieldvalue);
                    fieldname.clear();
                    fieldvalue.clear();
                    begin += 2;
                    state = HeaderEnd;
                } else
                    throw ResponseData(400);
                break;
            case FieldContent:
                if (RequestUtility::isFieldVchar(begin)) {
                    fieldvalue += *begin;
                    begin++;
                } else if (RequestUtility::isWS(begin)) {
                    state = WhiteSpace;
                } else
                    state = FieldValue;
                break;
            case ObsFold:
                fieldvalue += std::string(begin, begin + 2);
                state = WhiteSpace;
                begin += 2;
                break;
            case HeaderEnd:
                if (RequestUtility::isCRLF(begin)) {
                    begin += 2;
                    return;
                } else
                    state = FieldName;
                break;
        }
    }
}

void Request::parseBodyByContentLength(RequestData& requestData) {
    std::string length = requestData.header["content-length"];
    long long bodyLength = RequestUtility::strtonum(length);

    requestData.body = std::string(requestData.rawData.begin(),
                                   requestData.rawData.begin() + bodyLength);
}

void Request::parseBodyByTransferEncoding(RequestData& requestData) {
    ChunkState state = Chunk;
    std::string buffer, chunkData, chunkSizeStr;
    std::string trailerFieldName, trailerFieldValue;
    long long chunkSizeNum;
    StrIter begin = requestData.rawData.begin();

    while (true) {
        switch (state) {
            case Chunk:
                if (*begin == '0') {
                    state = LastChunk;
                } else if (RequestUtility::isHexDigit(begin)) {
                    state = ChunkSize;
                } else
                    throw ResponseData(400);
                break;
            case ChunkSize:
                if (RequestUtility::isHexDigit(begin)) {
                    chunkSizeStr += *begin;
                } else if (*begin == ';') {
                    chunkSizeNum = RequestUtility::hexTonum(chunkSizeStr);
                    state = ChunkExt;
                } else if (RequestUtility::isCRLF(begin)) {
                    chunkSizeNum = RequestUtility::hexTonum(chunkSizeStr);
                    begin++;
                    state = ChunkData;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case ChunkExt:
                if (RequestUtility::isCRLF(begin)) {
                    begin += 2;
                    state = ChunkData;
                } else
                    begin++;
                break;
            case ChunkData:
                if (RequestUtility::isCRLF(begin)) {
                    if (static_cast<long long>(chunkData.size()) ==
                        chunkSizeNum) {
                        buffer += chunkData;
                        chunkSizeStr.clear();
                        chunkData.clear();
                    } else
                        throw ResponseData(400);
                    begin += 2;
                    state = Chunk;
                } else {
                    chunkData += *begin;
                    begin++;
                }
                break;
            case LastChunk:
                if (*begin == '0')
                    begin++;
                else if (*begin == ';') {
                    state = LastChunkExt;
                } else if (RequestUtility::isCRLF(begin)) {
                    begin += 2;
                    state = TrailerStart;
                } else
                    throw ResponseData(400);
                break;
            case LastChunkExt:
                if (RequestUtility::isCRLF(begin)) {
                    begin += 2;
                    state = TrailerStart;
                } else
                    begin++;
                break;
            case TrailerStart:
                if (RequestUtility::isTchar(begin)) {
                    state = TrailerFieldName;
                } else if (RequestUtility::isCRLF(begin)) {
                    state = TrailerEnd;
                } else
                    throw ResponseData(400);
                break;
            case TrailerFieldName:
                if (RequestUtility::isTchar(begin)) {
                    trailerFieldName += std::tolower(*begin);
                } else if (*begin == ':') {
                    state = TrailerWhiteSpace;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case TrailerWhiteSpace:
                if (RequestUtility::isWS(begin)) {
                    trailerFieldValue += *begin;
                    begin++;
                } else
                    state = TrailerFieldValue;
                break;
            case TrailerFieldValue:
                if (RequestUtility::isFieldVchar(begin)) {
                    state = TrailerFieldContent;
                } else if (RequestUtility::isObsFold(begin)) {
                    state = TrailerObsFold;
                } else if (RequestUtility::isCRLF(begin)) {
                    trailerFieldValue =
                        RequestUtility::th_strtrim(trailerFieldValue, ' ');
                    requestData.bodyHeader[trailerFieldName] =
                        trailerFieldValue;
                    trailerFieldName.clear();
                    trailerFieldValue.clear();
                    begin += 2;
                    state = TrailerEnd;
                } else
                    throw ResponseData(400);
                break;
            case TrailerFieldContent:
                if (RequestUtility::isFieldVchar(begin)) {
                    trailerFieldValue += *begin;
                    begin++;
                } else if (RequestUtility::isWS(begin)) {
                    state = TrailerWhiteSpace;
                } else
                    state = TrailerFieldValue;
                break;
            case TrailerObsFold:
                trailerFieldValue += std::string(begin, begin + 2);
                state = TrailerWhiteSpace;
                begin += 2;
                break;
            case TrailerEnd:
                if (RequestUtility::isCRLF(begin)) {
                    begin += 2;
                    requestData.body = buffer;
                } else
                    state = TrailerFieldName;
                break;
        }
    }
}
