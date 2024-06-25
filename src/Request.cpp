#include "Request.hpp"

#include <cstdlib>
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
    uchar *begin, *end, input;

    state = RequestLineStart;
    begin = reinterpret_cast<uchar*>(&requestMessage.front());
    end = reinterpret_cast<uchar*>(&requestMessage.back() + 1);
    while (begin != end) {
        input = *begin;
        switch (state) {
            case RequestLineStart:
                if (RequestUtility::isTchar(input)) {
                    state = Method;
                } else
                    throw ResponseData(400);
                break;
            case Method:
                if (RequestUtility::isTchar(input)) {
                    token += input;
                } else if (input == ' ') {
                    state = MethodEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case MethodEnd:
                requestData.setMethod(token);
                token = "";
                state = RequestTargetStart;
            case RequestTargetStart:
                if (input == '/') {
                    state = AbsolutePath;
                } else
                    throw ResponseData(400);
                break;
            case AbsolutePath:
                if (RequestUtility::isPchar(begin) || input == '/') {
                    token += input;
                    if (input == '%') {
                        token += *(++begin);
                        token += *(++begin);
                    }
                } else if (input == '?') {
                    requestData.setPath(token);
                    token += input;
                    queryStart = token.size();
                    state = Query;
                } else if (input == ' ') {
                    requestData.setPath(token);
                    state = RequestTargetEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case Query:
                if (RequestUtility::isPchar(begin) || input == '/' ||
                    input == '?') {
                    token += input;
                    if (input == '%') {
                        token += *(++begin);
                        token += *(++begin);
                    }
                } else if (input == ' ') {
                    requestData.setPath(token.substr(queryStart));
                    state = RequestTargetEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case RequestTargetEnd:
                requestData.setRequestTarget(token);
                token = "";
                state = HTTPVersion;
                break;
            case HTTPVersion:
                if (RequestUtility::isHttpVersion(begin)) {
                    token = RequestUtility::th_substr(begin, 0, 8);
                    requestData.setVersion(token);
                    token = "";
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
                if (RequestUtility::isTchar(input)) {
                    state = FieldName;
                } else if (RequestUtility::isCRLF(begin))
                    state = HeaderEnd;
                else
                    throw ResponseData(400);
                break;
            case FieldName:
                if (RequestUtility::isTchar(input)) {
                    fieldname += tolower(input);
                } else if (input == ':') {
                    state = WhiteSpace;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case WhiteSpace:
                if (RequestUtility::isWS(input)) {
                    fieldvalue += input;
                    begin++;
                } else
                    state = FieldValue;
                break;
            case FieldValue:
                if (RequestUtility::isFieldVchar(input)) {
                    state = FieldContent;
                } else if (RequestUtility::isObsFold(begin)) {
                    state = ObsFold;
                } else if (RequestUtility::isCRLF(begin)) {
                    fieldvalue = RequestUtility::th_strtrim(fieldvalue, ' ');
                    requestData.setHeader(fieldname, fieldvalue);
                    fieldname = "";
                    fieldvalue = "";
                    begin += 2;
                    state = HeaderEnd;
                } else
                    throw ResponseData(400);
                break;
            case FieldContent:
                if (RequestUtility::isFieldVchar(input)) {
                    fieldvalue += input;
                    begin++;
                } else if (RequestUtility::isWS(input)) {
                    state = WhiteSpace;
                } else
                    state = FieldValue;
                break;
            case ObsFold:
                fieldvalue += input;
                fieldvalue += *(++begin);
                state = WhiteSpace;
                begin++;
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
                if (RequestUtility::doesValidContentLength(bodyHeaderValue)) {
                    token = parseBodyByContentLength(begin, bodyHeaderValue);
                    requestData.setBody(token);
                    token = "";
                    state = BodyEnd;
                } else
                    throw ResponseData(400);
                break;
            case TransferEncoding:
                bodyHeaderValue = requestData.header[bodyHeaderName];
                if (bodyHeaderValue == "chunked") {
                    token = parseBodyByTransferEncoding(begin, end);
                    requestData.setBody(token);
                    token = "";
                    state = BodyEnd;
                } else
                    throw ResponseData(400);
                break;
            case BodyEnd:
                return;
        }
    }
}

std::string Request::parseBodyByContentLength(uchar* begin,
                                              std::string length) {
    std::istringstream bodyStream(std::string(reinterpret_cast<char*>(begin)));
    Configuration config = Configuration::getInstance();
    std::string buffer;
    long long bodyLength = strtoll(length.c_str(), NULL, 10);

    if (bodyStream.fail()) {
        throw ResponseData(400);  // stream 생성 실패에 대한 throw
    } else if (bodyLength < 0)
        throw ResponseData(400);  // content-length가 음수일 때 throw
    bodyStream.read(&buffer[0], bodyLength);
    if (bodyStream.gcount() != bodyLength) {
        throw ResponseData(400);  // read error 로 인한 throw
    }
    return buffer;
}

std::string Request::parseBodyByTransferEncoding(uchar* begin, uchar* end) {
    ChunkState state;
    std::string buffer;
    uchar input;

    state = Chunk;
    while (begin != end) {
        input = *begin;
        switch (state) {
            case Chunk:
                if (input == '0') {
                    state = LastChunk;
                } else if (RequestUtility::isHexDigit(input)) {
                    state = ChunkSize;
                } else
                    throw ResponseData(400);
                break;
            case ChunkSize:
                if (RequestUtility::isHexDigit(input)) {
                    buffer += input;
                } else if (input == ';') {
                    state = ChunkExt;
                } else if (RequestUtility::isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    state = ChunkData;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case ChunkExt:
                if (RequestUtility::isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    state = ChunkData;
                }
                begin++;
                break;
            case ChunkData:
                buffer += input;
                if (RequestUtility::isCRLF(begin)) {
                    buffer += *(++begin);
                    state = Chunk;
                }
                begin++;
                break;
            case LastChunk:
                if (input == '0') {
                    buffer += input;
                } else if (input == ';') {
                    state = LastChunkExt;
                } else if (RequestUtility::isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    state = TrailerStart;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case LastChunkExt:
                if (RequestUtility::isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    state = TrailerStart;
                }
                begin++;
                break;
            case TrailerStart:
                if (RequestUtility::isTchar(input)) {
                    state = TrailerFieldName;
                } else if (RequestUtility::isCRLF(begin)) {
                    state = TrailerEnd;
                } else
                    throw ResponseData(400);
                break;
            case TrailerFieldName:
                if (input == ':') {
                    state = TrailerWhiteSpace;
                } else if (!RequestUtility::isTchar(input)) {
                    throw ResponseData(400);
                }
                buffer += input;
                begin++;
                break;
            case TrailerWhiteSpace:
                if (RequestUtility::isWS(input)) {
                    buffer += input;
                    begin++;
                } else
                    state = TrailerFieldValue;
                break;
            case TrailerFieldValue:
                if (RequestUtility::isFieldVchar(input)) {
                    state = TrailerFieldContent;
                } else if (RequestUtility::isObsFold(begin)) {
                    state = TrailerObsFold;
                } else if (RequestUtility::isCRLF(begin) &&
                           RequestUtility::isCRLF(begin + 2)) {
                    buffer += input;
                    buffer += *(++begin);
                    begin++;
                    state = TrailerEnd;
                } else if (RequestUtility::isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    begin++;
                    state = TrailerFieldName;
                } else
                    throw ResponseData(400);
                break;
            case TrailerFieldContent:
                if (RequestUtility::isFieldVchar(input)) {
                    buffer += input;
                    begin++;
                } else if (RequestUtility::isWS(input)) {
                    state = TrailerWhiteSpace;
                } else
                    state = TrailerFieldValue;
                break;
            case TrailerObsFold:
                buffer += input;
                buffer += *(++begin);
                state = TrailerWhiteSpace;
                begin++;
                break;
            case TrailerEnd:
                buffer += input;
                buffer += *(++begin);
                begin++;
                break;
        }
    }
    return buffer;
}
