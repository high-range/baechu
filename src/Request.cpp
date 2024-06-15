#include "Request.hpp"

#include "Configuration.hpp"
#include "RequestData.hpp"
#include "ResponseData.hpp"

void Request::messageParse(std::string& requestMessage,
                           RequestData& requestData,
                           const Configuration& configuration) {
    State state = StartLineStart;
    std::string token;
    std::string fieldname;
    std::string fieldvalue;
    std::string::size_type queryStart;
    unsigned char* begin;
    unsigned char* end;
    unsigned char input;

    (void)configuration;
    // test를 위해 임시로 작성

    begin = reinterpret_cast<unsigned char*>(&requestMessage.front());
    end = reinterpret_cast<unsigned char*>(&requestMessage.back() + 1);
    while (begin != end) {
        input = *begin;
        switch (state) {
            case StartLineStart:
                if (isTchar(input)) {
                    state = Method;
                } else
                    throw ResponseData(400);
                break;
            case Method:
                if (isTchar(input)) {
                    token += input;
                } else if (input == ' ') {
                    state = MethodEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case MethodEnd:
                requestData.startLine.method = token;
                token = "";
                state = RequestTargetStart;
            case RequestTargetStart:
                if (input == '/') {
                    state = AbsolutePath;
                } else
                    throw ResponseData(400);
                break;
            case AbsolutePath:
                if (isPchar(begin) || input == '/') {
                    token += input;
                    if (input == '%') {
                        token += *(++begin);
                        token += *(++begin);
                    }
                } else if (input == '?') {
                    requestData.startLine.path = token;
                    token += input;
                    queryStart = token.size();
                    state = Query;
                } else if (input == ' ') {
                    requestData.startLine.path = token;
                    state = RequestTargetEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case Query:
                if (isPchar(begin) || input == '/' || input == '?') {
                    token += input;
                    if (input == '%') {
                        token += *(++begin);
                        token += *(++begin);
                    }
                } else if (input == ' ') {
                    requestData.startLine.query = token.substr(queryStart);
                    state = RequestTargetEnd;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case RequestTargetEnd:
                requestData.startLine.requestTarget = token;
                token = "";
                state = HTTPVersion;
                break;
            case HTTPVersion:
                if (isHttpVersion(begin)) {
                    requestData.startLine.version = th_substr(begin, 0, 8);
                    begin += 8;
                    state = StartLineEnd;
                } else
                    throw ResponseData(400);
                break;
            case StartLineEnd:
                if (isCRLF(begin)) {
                    state = HeaderStart;
                    begin += 2;
                } else
                    throw ResponseData(400);
                break;
            case HeaderStart:
                if (isTchar(input)) {
                    state = FieldName;
                } else if (isCRLF(begin))
                    state = HeaderEnd;
                else
                    throw ResponseData(400);
                break;
            case FieldName:
                if (isTchar(input)) {
                    fieldname += tolower(input);
                } else if (input == ':') {
                    state = WhiteSpace;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case WhiteSpace:
                if (isWS(input)) {
                    fieldvalue += input;
                    begin++;
                } else
                    state = FieldValue;
                break;
            case FieldValue:
                if (isFieldVchar(input)) {
                    state = FieldContent;
                } else if (isObsFold(begin)) {
                    state = ObsFold;
                } else if (isCRLF(begin) && isCRLF(begin + 2)) {
                    state = HeaderEnd;
                    begin += 2;
                } else if (isCRLF(begin)) {
                    if (requestData.header[fieldname].empty()) {
                        requestData.header[fieldname] =
                            th_strtrim(fieldvalue, ' ');
                    } else
                        requestData.header[fieldname] +=
                            ", " + th_strtrim(fieldvalue, ' ');
                    fieldname = "";
                    fieldvalue = "";
                    state = FieldName;
                    begin += 2;
                } else
                    throw ResponseData(400);
                break;
            case FieldContent:
                if (isFieldVchar(input)) {
                    fieldvalue += input;
                    begin++;
                } else if (isWS(input)) {
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
                begin += 2;
                state = BodyStart;
                break;
            case BodyStart:
                if (doesExistContentLength(requestData.header) &&
                    doesExistTransferEncoding(requestData.header)) {
                    throw ResponseData(400);
                } else if (doesExistContentLength(requestData.header)) {
                    state = ContentLength;
                } else if (doesExistTransferEncoding(requestData.header)) {
                    state = TransferEncoding;
                } else
                    state = BodyEnd;
                break;
            case ContentLength:
                if (doesValidContentLength(
                        requestData.header["Content-Length"])) {
                    requestData.body = contentLengthBodyParse(
                        begin, requestData.header["Content-Length"]);
                    state = BodyEnd;
                } else
                    throw ResponseData(400);
                break;
            case TransferEncoding:
                if (requestData.header["Transfer-Encoding"] == "chunked") {
                    requestData.body = transferEncodingBodyParse(begin, end);
                    state = BodyEnd;
                } else
                    throw ResponseData(400);
                break;
            case BodyEnd:
                return;
        }
    }
}

std::string Request::contentLengthBodyParse(unsigned char* begin,
                                            std::string length) {
    std::istringstream bodyStream(std::string(reinterpret_cast<char*>(begin)));
    long long bodyLength = std::stoll(length);  // stoll 함수 변경해야 함
    std::string buffer(bodyLength, '\0');

    if (bodyStream.fail()) {
        throw ResponseData(400);  // stream 생성 실패에 대한 throw
    }
    bodyStream.read(&buffer[0], bodyLength);
    if (bodyStream.gcount() != bodyLength) {
        throw ResponseData(400);  // read error 로 인한 throw
    }
    return buffer;
}

std::string Request::transferEncodingBodyParse(unsigned char* begin,
                                               unsigned char* end) {
    unsigned char input;
    std::string buffer;
    ChunkState state = Chunk;

    while (begin != end) {
        input = *begin;
        switch (state) {
            case Chunk:
                if (input == '0') {
                    state = LastChunk;
                } else if (isHexDigit(input)) {
                    state = ChunkSize;
                } else
                    throw ResponseData(400);
                break;
            case ChunkSize:
                if (isHexDigit(input)) {
                    buffer += input;
                } else if (input == ';') {
                    state = ChunkExt;
                } else if (isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    state = ChunkData;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case ChunkExt:
                if (isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    state = ChunkData;
                }
                begin++;
                break;
            case ChunkData:
                buffer += input;
                if (isCRLF(begin)) {
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
                } else if (isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    state = TrailerStart;
                } else
                    throw ResponseData(400);
                begin++;
                break;
            case LastChunkExt:
                if (isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    state = TrailerStart;
                }
                begin++;
                break;
            case TrailerStart:
                if (isTchar(input)) {
                    state = TrailerFieldName;
                } else if (isCRLF(begin)) {
                    state = TrailerEnd;
                } else
                    throw ResponseData(400);
                break;
            case TrailerFieldName:
                if (input == ':') {
                    state = TrailerWhiteSpace;
                } else if (!isTchar(input)) {
                    throw ResponseData(400);
                }
                buffer += input;
                begin++;
                break;
            case TrailerWhiteSpace:
                if (isWS(input)) {
                    buffer += input;
                    begin++;
                } else
                    state = TrailerFieldValue;
                break;
            case TrailerFieldValue:
                if (isFieldVchar(input)) {
                    state = TrailerFieldContent;
                } else if (isObsFold(begin)) {
                    state = TrailerObsFold;
                } else if (isCRLF(begin) && isCRLF(begin + 2)) {
                    buffer += input;
                    buffer += *(++begin);
                    begin++;
                    state = TrailerEnd;
                } else if (isCRLF(begin)) {
                    buffer += input;
                    buffer += *(++begin);
                    begin++;
                    state = TrailerFieldName;
                } else
                    throw ResponseData(400);
                break;
            case TrailerFieldContent:
                if (isFieldVchar(input)) {
                    buffer += input;
                    begin++;
                } else if (isWS(input)) {
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

bool Request::isObsFold(const unsigned char* str) {
    return isCRLF(str) && isWS(str[2]);
}

bool Request::isWS(const unsigned char c) { return c == ' ' || c == '\t'; }

bool Request::isFieldVchar(const unsigned char c) {
    return isgraph(c) || (c >= 128 && c <= 255);
}

bool Request::isPchar(const unsigned char* str) {
    std::string subDelims = "!$&'()*+,;=";

    if (str[0] == '%' && isHexDigit(str[1]) && isHexDigit(str[2])) {
        return true;
    }  // pct-encoded check;
    if (isalpha(str[0]) || isdigit(str[0]) || str[0] == '-' || str[0] == '.' ||
        str[0] == '_' || str[0] == '~' || str[0] == ':' || str[0] == '@') {
        return true;
    }  // unreserved, ':', '@' check
    for (size_t i = 0; i < subDelims.size(); i++) {
        if (str[0] == subDelims[i]) {
            return true;
        }
    }  // sub-delims check
    return false;
}

bool Request::isHexDigit(const unsigned char c) {
    std::string HexAlpha = "ABCDEF";

    if (isdigit(c)) {
        return true;
    }
    for (int i = 0; HexAlpha[i] != '\0'; i++) {
        if (c == HexAlpha[i]) {
            return true;
        }
    }
    return false;
}

bool Request::isHttpVersion(const unsigned char* str) {
    return str[0] == 'H' && str[1] == 'T' && str[2] == 'T' && str[3] == 'P' &&
           str[4] == '/' && isdigit(str[5]) && str[6] == '.' && isdigit(str[7]);
}

bool Request::isTchar(const unsigned char c) {
    std::string delimiter = "(),/:;<=>?@[\\]{}";

    if (!isgraph(c) || c == '\"') {
        return false;
    }
    for (size_t i = 0; i < delimiter.size(); i++) {
        if (c == delimiter[i]) {
            return false;
        }
    }
    return true;
}

bool Request::isCRLF(const unsigned char* str) {
    return str[0] == '\r' && str[1] == '\n';
}

std::string Request::th_substr(const unsigned char* src, const size_t start,
                               const size_t end) {
    std::string result;

    if (start >= end) {
        return result;
    }
    for (size_t i = start; i < end; i++) {
        result += src[i];
    }
    return result;
}

std::string Request::th_strtrim(const std::string& src,
                                const unsigned char target) {
    size_t start = 0;
    size_t end = 0;

    for (size_t i = 0; i < src.size(); i++) {
        if (src[i] != target) {
            start = i;
            break;
        }
    }
    for (size_t i = src.size() - 1; i > -1; i--) {
        if (src[i] != target) {
            end = i;
            break;
        }
    }
    if (start <= end) {
        return 0;
    }
    return src.substr(start, end - start);
}

bool Request::doesValidContentLength(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    for (size_t i = 0; i < str.size(); i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool Request::doesExistContentLength(
    const std::map<std::string, std::string>& header) {
    return header.find("Content-Length") != header.end();
}

bool Request::doesExistTransferEncoding(
    const std::map<std::string, std::string>& header) {
    return header.find("Transfer-Encoding") != header.end();
}
