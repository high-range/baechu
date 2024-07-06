#include "RequestUtility.hpp"

#include <sstream>

bool RequestUtility::isObsFold(StrIter begin) {
    return isCRLF(begin) && isWS(begin + 2);
}

bool RequestUtility::isWS(StrIter begin) {
    return *begin == ' ' || *begin == '\t';
}

bool RequestUtility::isFieldVchar(StrIter begin) { return isgraph(*begin); }

bool RequestUtility::isPchar(StrIter begin) {
    std::string subDelims = "!$&'()*+,;=";

    if (*begin == '%' && isHexDigit(begin + 1) && isHexDigit(begin + 2)) {
        return true;
    }  // pct-encoded check;
    if (isalpha(*begin) || isdigit(*begin) || *begin == '-' || *begin == '.' ||
        *begin == '_' || *begin == '~' || *begin == ':' || *begin == '@') {
        return true;
    }  // unreserved, ':', '@' check
    for (size_t i = 0; i < subDelims.size(); i++) {
        if (*begin == subDelims[i]) {
            return true;
        }
    }  // sub-delims check
    return false;
}

bool RequestUtility::isHexDigit(StrIter begin) {
    std::string HexBase = "0123456789ABCDEFabcdef";

    for (size_t i = 0; i < HexBase.size(); i++) {
        if (*begin == HexBase[i]) {
            return true;
        }
    }
    return false;
}

bool RequestUtility::isHttpVersion(StrIter begin) {
    return std::string(begin, begin + 8) == "HTTP/1.1";
}

bool RequestUtility::isTchar(StrIter begin) {
    std::string delimiter = "(),/:;<=>?@[\\]{}";

    if (!isgraph(*begin) || *begin == '\"') {
        return false;
    }
    for (size_t i = 0; i < delimiter.size(); i++) {
        if (*begin == delimiter[i]) {
            return false;
        }
    }
    return true;
}

bool RequestUtility::isCRLF(StrIter begin) {
    return *begin == '\r' && *(begin + 1) == '\n';
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
        return "";
    }
    return src.substr(start, end - start);
}

bool RequestUtility::isNum(const std::string& str) {
    if (str.empty() || (str.size() > 1 && str[0] == '0')) {
        return false;
    }
    for (size_t i = 0; i < str.size(); i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

long long RequestUtility::strtonum(const std::string& str) {
    std::stringstream ss(str);
    long long num;

    if (ss.fail()) {
        return -1;
    }
    ss >> num;
    if (str.back() == 'M') {
        num *= 1024 * 1024;
    }
    return num;
}

long long RequestUtility::hexTonum(const std::string& str) {
    std::stringstream ss;
    long long num;

    ss << std::hex << str;
    ss >> num;
    return num;
}

bool RequestUtility::isHeaderComplete(const std::string& rawData) {
    return rawData.find("\r\n\r\n") != std::string::npos;
}
