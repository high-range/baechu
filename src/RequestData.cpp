#include "RequestData.hpp"

std::string RequestData::getMethod() const { return startLine.method; };
std::string RequestData::getRequestTarget() const {
    return startLine.requestTarget;
};
std::string RequestData::getPath() const { return startLine.path; };
std::string RequestData::getQuery() const { return startLine.query; };
std::string RequestData::getVersion() const { return startLine.version; };
Header RequestData::getHeader() const { return header; };
std::string RequestData::getBody() const { return body; };
std::string RequestData::getPort() const { return clientData.port; };
std::string RequestData::getIP() const { return clientData.ip; };

const std::string RequestData::getBodyHeaderName() {
    Header::iterator contentLength;
    Header::iterator transferEncoding;

    contentLength = header.find("content-length");
    transferEncoding = header.find("transfer-encoding");
    if (contentLength != header.end() && transferEncoding != header.end())
        return "Error";
    else if (contentLength != header.end())
        return "content-length";
    else if (transferEncoding != header.end())
        return "transfer-encoding";
    return "";
}

void RequestData::setMethod(const std::string& method) {
    this->startLine.method = method;
}

void RequestData::setRequestTarget(const std::string& requestTarget) {
    this->startLine.requestTarget = requestTarget;
}

void RequestData::setPath(const std::string& path) {
    this->startLine.path = path;
}

void RequestData::setQuery(const std::string& query) {
    this->startLine.query = query;
}

void RequestData::setVersion(const std::string& version) {
    this->startLine.version = version;
}

void RequestData::setHeader(const std::string& key, const std::string& value) {
    if (header.find(key) == header.end()) {
        header[key] = value;
    } else
        header[key] += ", " + value;
}

void RequestData::setBody(const std::string& body) { this->body = body; }

void RequestData::setPort(const std::string& port) {
    this->clientData.port = port;
}

void RequestData::setIP(const std::string& ip) { this->clientData.ip = ip; }
