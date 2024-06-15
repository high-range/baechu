#include "RequestData.hpp"

std::string RequestData::getMethod() const { return startLine.method; };
std::string RequestData::getPath() const { return startLine.path; };
std::string RequestData::getVersion() const { return startLine.version; };
std::map<std::string, std::string> RequestData::getHeader() const {
    return header;
};
std::string RequestData::getBody() const { return body; };
std::string RequestData::getQuery() const {
    if (startLine.path.find('?') != std::string::npos) {
        return startLine.path.substr(startLine.path.find('?') + 1);
    } else
        return NULL;
}
