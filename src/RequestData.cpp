#include "RequestData.hpp"

std::string RequestData::getMethod() const { return startLine.method; };
std::string RequestData::getPath() const { return startLine.path; };
std::string RequestData::getVersion() const { return startLine.version; };
std::map<std::string, std::vector<std::string> > RequestData::getHeader()
    const {
    return header;
};
std::string RequestData::getBody() const { return body; };
