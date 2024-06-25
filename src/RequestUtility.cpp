#include "RequestUtility.hpp"

bool RequestUtility::isObsFold(c_uchar* str) {
    return isCRLF(str) && isWS(str[2]);
}

bool RequestUtility::isWS(c_uchar c) { return c == ' ' || c == '\t'; }

bool RequestUtility::isFieldVchar(c_uchar c) {
    return isgraph(c) || (c >= 128 && c <= 255);
}

bool RequestUtility::isPchar(c_uchar* str) {
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

bool RequestUtility::isHexDigit(c_uchar c) {
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

bool RequestUtility::isHttpVersion(c_uchar* str) {
    return str[0] == 'H' && str[1] == 'T' && str[2] == 'T' && str[3] == 'P' &&
           str[4] == '/' && isdigit(str[5]) && str[6] == '.' && isdigit(str[7]);
}

bool RequestUtility::isTchar(c_uchar c) {
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

bool RequestUtility::isCRLF(c_uchar* str) {
    return str[0] == '\r' && str[1] == '\n';
}

std::string RequestUtility::th_substr(c_uchar* src, const size_t start,
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

std::string RequestUtility::th_strtrim(const std::string& src, c_uchar target) {
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

bool RequestUtility::doesValidContentLength(const std::string& str) {
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
