#include "Request.hpp"

#include "Configuration.hpp"
#include "RequestData.hpp"

void Request::messageParse(std::string& requestMessage,
                           RequestData& requestData,
                           const Configuration& configuration) {
    State state = StartLineStart;
    std::string token;
    std::string fieldname;
    std::string fieldvalue;
    char* begin;
    char* end;
    unsigned char input;

    (void)configuration;
    // test를 위해 임시로 작성

    begin = &requestMessage.front();
    end = &requestMessage.back();
    while (begin != end) {
        input = *begin;
        switch (state) {
            case StartLineStart:
                if (isTchar(input)) {
                    state = Method;
                } else
                    throw;
                break;
            case Method:
                if (isTchar(input)) {
                    token += input;
                } else if (input == ' ') {
                    state = MethodEnd;
                } else
                    throw;
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
                    throw;  // syntax error
                break;
            case AbsolutePath:
                if (isPchar(begin) || input == '/') {
                    token += input;
                    if (input == '%') {
                        token += *(++begin);
                        token += *(++begin);
                    }
                } else if (input == '?') {
                    state = Query;
                } else if (input == ' ') {
                    state = RequestTargetEnd;
                }
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
                    state = RequestTargetEnd;
                } else
                    throw;  // lexical error
                begin++;
            case RequestTargetEnd:
                requestData.startLine.path = token;
                token = "";
                state = HTTPVersion;
                break;
            case HTTPVersion:
                if (isHttpVersion(begin)) {
                    requestData.startLine.version = th_substr(begin, 0, 8);
                    begin += 8;
                    state = StartLineEnd;
                } else
                    throw;
                break;
            case StartLineEnd:
                if (isCRLF(begin)) {
                    state = HeaderStart;
                    begin += 2;
                } else
                    throw;
                break;
            case HeaderStart:
                if (isTchar(input)) {
                    state = FieldName;
                } else if (isCRLF(begin))
                    state = HeaderEnd;
                else
                    throw;
                break;
            case FieldName:
                if (isTchar(input)) {
                    fieldname += input;
                } else if (input == ':') {
                    state = WhiteSpace;
                }
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
                    requestData.header[fieldname].push_back(
                        th_strtrim(fieldvalue, ' '));
                    fieldname = "";
                    fieldvalue = "";
                    state = FieldName;
                    begin += 2;
                } else
                    throw;
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
                state = Body;
                break;
            case Body:
                return;
        }
    }
}

bool Request::isObsFold(const char* str) { return isCRLF(str) && isWS(str[2]); }

bool Request::isWS(const unsigned char c) { return c == ' ' || c == '\t'; }

bool Request::isFieldVchar(const unsigned char c) {
    return isgraph(c) || (c >= 128 && c <= 255);
}

bool Request::isPchar(const char* str) {
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

bool Request::isHexDigit(const char c) {
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

bool Request::isHttpVersion(const char* str) {
    return str[0] == 'H' && str[1] == 'T' && str[2] == 'T' && str[3] == 'P' &&
           str[4] == '/' && isdigit(str[5]) && str[6] == '.' && isdigit(str[7]);
}

bool Request::isTchar(const char c) {
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

bool Request::isCRLF(const char* str) {
    return str[0] == '\r' && str[1] == '\n';
}

std::string Request::th_substr(const char* src, const size_t start,
                               const size_t end) {
    std::string result;

    if (start <= end) {
        return result;
    }
    for (size_t i = start; i < end; i++) {
        result += src[i];
    }
    return result;
}

std::string Request::th_strtrim(const std::string& src, const char target) {
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
