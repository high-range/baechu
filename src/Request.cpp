#include "Request.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "RequestData.hpp"
#include "RequestUtility.hpp"
#include "ResponseData.hpp"

void Request::parseMessage(std::string& requestMessage,
                           RequestData& requestData) {
    State state;
    size_t queryStart;
    std::string token;
    std::string fieldname, fieldvalue;
    std::string bodyHeaderName, bodyHeaderValue;
    StrIter begin, end;

    state = RequestLineStart;
    begin = requestMessage.begin();
    end = requestMessage.end();
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
                    state = BodyStart;
                } else
                    state = FieldName;
                break;
            case BodyStart:
                bodyHeaderName = requestData.getBodyHeaderName();
                if (bodyHeaderName == "content-length") {
                    state = ContentLength;
                } else if (bodyHeaderName == "transfer-encoding") {
                    state = TransferEncoding;
                } else if (bodyHeaderName == "") {
                    state = BodyEnd;
                } else
                    throw ResponseData(400);
                break;
            case ContentLength:
                bodyHeaderValue = requestData.header[bodyHeaderName];
                if (RequestUtility::isNum(bodyHeaderValue)) {
                    token = parseBodyByContentLength(std::string(begin, end),
                                                     bodyHeaderValue);
                    requestData.setBody(token);
                    token.clear();
                    state = BodyEnd;
                } else
                    throw ResponseData(400);
                break;
            case TransferEncoding:
                bodyHeaderValue = requestData.header[bodyHeaderName];
                if (bodyHeaderValue == "chunked") {
                    token = parseBodyByTransferEncoding(std::string(begin, end),
                                                        requestData.bodyHeader);
                    requestData.setBody(token);
                    token.clear();
                    state = BodyEnd;
                } else
                    throw ResponseData(400);
                break;
            case BodyEnd:
                return;
        }
    }
}

std::string Request::parseBodyByContentLength(std::string body,
                                              std::string length) {
    std::stringstream bodyStream(body);
    long long bodyLength = RequestUtility::strtonum(length);
    // Configuration config = Configuration::getInstance();

    if (bodyStream.fail()) {
        throw ResponseData(400);
    } else if (bodyLength < 0)
        throw ResponseData(400);
    std::string buffer(bodyLength, '\0');
    bodyStream.read(&buffer[0], bodyLength);
    if (body.size() != buffer.size()) {
        throw ResponseData(400);
    }
    // if (bodyStream.gcount() != bodyLength) {
    //     throw ResponseData(400);
    // } // max body size 관련 처리 필요, 일단 주석처리
    return buffer;
}

std::string Request::parseBodyByTransferEncoding(std::string body,
                                                 Header& bodyHeader) {
    ChunkState state = Chunk;
    std::string buffer, chunkData, chunkSizeStr;
    std::string trailerFieldName, trailerFieldValue;
    long long chunkSizeNum;
    StrIter begin = body.begin();

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
                    bodyHeader[trailerFieldName] = trailerFieldValue;
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
                    return buffer;
                } else
                    state = TrailerFieldName;
                break;
        }
    }
    return "";
}
