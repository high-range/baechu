#include "Request.hpp"

#include "Configuration.hpp"
#include "RequestData.hpp"

void Request::MessageParse(std::string& requestMessage,
                           RequestData& requestData,
                           const Configuration& configuration) {
    State state;
    std::string token;
    char input;
    char* begin;
    char* end;

    begin = &requestMessage.front();
    end = &requestMessage.back();
}

bool Request::isTChar(const char c) {
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

bool Request::isVChar(const char c) { return (isgraph(c)); }
// Visible character(except SP) check

bool Request::isWS(const char c) { return (c == ' ' || c == 9); }
// Space and Horizontal tab check

bool Request::isCRLF(const char now, const char next) {
    return (now == '\r' && next == '\n');
}
// Carriage return and Line feed check
