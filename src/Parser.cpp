#include "Parser.hpp"

void Parser::startLineParse(std::string& requestMessage,
                            RequestData& requestData) {
    startLineTokenize(requestMessage, requestData);
    startLineSyntaxCheck(requestData);
    startLineSemanticCheck(requestData);
    // Start line tokenization and syntax check
}

void Parser::headerParse(std::string& requestMessage,
                         RequestData& requestData) {
    headerTokenize(requestMessage, requestData);
    // Header tokenization and syntax check
}

void Parser::startLineTokenize(std::string& requestMessage,
                               RequestData& requestData) {
    StartLineState state;
    std::string token;
    char input;
    char* begin;
    char* end;

    state = Method;
    begin = &requestMessage.front();
    end = &requestMessage.back();

    while (begin != end) {
        input = *begin;
        switch (state) {
            case Method:
                if (input == ' ') {
                    requestData.startLine.method = token;
                    token = "";
                    state = Path;
                    begin++;
                } else {
                    token += input;
                    begin++;
                }
                break;
            case Path:
                if (input == ' ') {
                    requestData.startLine.path = token;
                    token = "";
                    state = Version;
                    begin++;
                } else {
                    token += input;
                    begin++;
                }
                break;
            case Version:
                if (isCRLF(input, *(begin + 1))) {
                    requestData.startLine.version = token;
                    return;
                } else {
                    token += input;
                    begin++;
                }
                break;
        }
    }
}  // Startline tokenization

void Parser::startLineSyntaxCheck(const RequestData& requestData) {
    for (size_t i = 0; i < requestData.startLine.method.length(); i++) {
        if (!isTchar(requestData.startLine.method[i])) {
            throw std::vector<std::pair<int, std::string> >(
                1, std::make_pair(400, "Bad Request"));
        }
    }
}

void Parser::startLineSemanticCheck(const RequestData& requestData) {
    if (requestData.startLine.method != "GET" &&
        requestData.startLine.method != "POST" &&
        requestData.startLine.method != "DELETE") {
        throw std::vector<std::pair<int, std::string> >(
            1, std::make_pair(501, "Not Implemented"));
    }
}

void Parser::headerTokenize(std::string& requestMessage,
                            RequestData& requestData) {
    HeaderState state;
    std::string fieldName;
    std::string fieldValue;
    char input;
    char* begin;
    char* end;

    state = FieldName;
    begin = &requestMessage[requestMessage.find("\r\n") + 2];
    end = &requestMessage.back();

    while (begin != end) {
        input = *begin;
        switch (state) {
            case FieldName:
                if (isCRLF(input, *(begin + 1))) {
                    state = End;
                    ++(++begin);
                } else if (isTchar(input)) {
                    fieldName += input;
                    begin++;
                } else if (input == ':') {
                    state = OWS;
                    begin++;
                } else {
                    throw std::invalid_argument("Invalid field name");
                }
                break;
            case OWS:
                if (isCRLF(input, *(begin + 1))) {
                    state = End;
                    ++(++begin);
                } else if (isWS(input)) {
                    begin++;
                } else {
                    state = FirstFieldContent;
                }
                break;
            case FirstFieldContent:
                if (isCRLF(input, *(begin + 1))) {
                    requestData.header[fieldName].push_back(fieldValue);
                    fieldName = "";
                    fieldValue = "";
                    state = End;
                    ++(++begin);
                } else if (isVChar(input)) {
                    fieldValue += input;
                    begin++;
                } else if (isWS(input)) {
                    fieldValue += " ";
                    state = RWS;
                } else {
                    throw std::invalid_argument("Invalid field content");
                }
                break;
            case RWS:
                if (isCRLF(input, *(begin + 1))) {
                    state = End;
                    ++(++begin);
                } else if (isWS(input)) {
                    begin++;
                } else {
                    state = SecondFieldContent;
                }
                break;
            case SecondFieldContent:
                if (isCRLF(input, *(begin + 1))) {
                    requestData.header[fieldName].push_back(fieldValue);
                    fieldName = "";
                    fieldValue = "";
                    state = End;
                    ++(++begin);
                } else if (isVChar(input)) {
                    fieldValue += input;
                    begin++;
                } else if (isWS(input)) {
                    requestData.header[fieldName].push_back(fieldValue);
                    fieldValue = "";
                    state = OWS;
                } else {
                    throw std::invalid_argument("Invalid field content");
                }
                break;
            case ObsFold:
                if (isCRLF(input, *(begin + 1))) {
                    state = End;
                    ++(++begin);
                } else if (isWS(input)) {
                    begin++;
                } else {
                    state = FirstFieldContent;
                }
                break;
            case End:
                if (isCRLF(input, *(begin + 1))) {
                    return;
                } else if (isWS(input)) {
                    state = ObsFold;
                } else {
                    state = FieldName;
                }
                break;
        }
    }
}
// Header tokenization

bool Parser::isTchar(const char c) {
    std::string delimiter = "(),/:;<=>?@[\\]{}\"";

    if (!isgraph(c)) {
        return false;
    }
    for (size_t i = 0; i < delimiter.length(); i++) {
        if (c == delimiter[i]) {
            return false;
        }
    }
    return true;
}
// Token character check

bool Parser::isVChar(const char c) {
    if (isgraph(c)) {
        return true;
    }
    return false;
}
// Visible character(except SP) check

bool Parser::isWS(const char c) {
    if (c == ' ' || c == 9) {
        return true;
    }
    return false;
}
// Space and Horizontal tab check

bool Parser::isCRLF(const char now, const char next) {
    if (now == '\r' && next == '\n') {
        return true;
    }
    return false;
}
// Carriage return and Line feed check
