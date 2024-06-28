#include "RequestData.hpp"

char* my_inet_ntoa(struct in_addr in) {
    static char buffer[INET_ADDRSTRLEN];

    unsigned char bytes[4];
    bytes[0] = (in.s_addr) & 0xFF;
    bytes[1] = (in.s_addr >> 8) & 0xFF;
    bytes[2] = (in.s_addr >> 16) & 0xFF;
    bytes[3] = (in.s_addr >> 24) & 0xFF;

    snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u", bytes[0], bytes[1],
             bytes[2], bytes[3]);
    return buffer;
}

RequestData::RequestData(sockaddr_in serverAddr, sockaddr_in clientAddr) {
    serverData.ip = my_inet_ntoa(serverAddr.sin_addr);
    serverData.port = std::to_string(ntohs(serverAddr.sin_port));
    clientData.ip = my_inet_ntoa(clientAddr.sin_addr);
    clientData.port = std::to_string(ntohs(clientAddr.sin_port));
}

std::string RequestData::getMethod() const { return startLine.method; };
std::string RequestData::getRequestTarget() const {
    return startLine.requestTarget;
};
std::string RequestData::getPath() const { return startLine.path; };
std::string RequestData::getQuery() const { return startLine.query; };
std::string RequestData::getVersion() const { return startLine.version; };
Header RequestData::getHeader() const { return header; };
std::string RequestData::getBody() const { return body; };
std::string RequestData::getServerPort() const { return serverData.port; };
std::string RequestData::getServerIP() const { return serverData.ip; };
std::string RequestData::getClientPort() const { return clientData.port; };
std::string RequestData::getClientIP() const { return clientData.ip; };

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
