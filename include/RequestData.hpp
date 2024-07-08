#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "RequestUtility.hpp"

typedef std::map<std::string, std::string> Header;

class RequestData {
  private:
    std::string rawData;
    ParseState state;

    // request message info
    struct StartLine {
        std::string method;
        std::string requestTarget;
        std::string path;
        std::string query;
        std::string version;
    } startLine;
    Header header;
    std::string body;
    Header bodyHeader;

    struct ClientData {
        std::string port;
        std::string ip;
    } clientData;

    struct ServerData {
        std::string port;
        std::string ip;
    } serverData;

    void setMethod(const std::string& method);
    void setRequestTarget(const std::string& requestTarget);
    void setPath(const std::string& path);
    void setQuery(const std::string& query);
    void setVersion(const std::string& version);
    void setHeader(const std::string& key, const std::string& value);
    void setBodyHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    const std::string getBodyHeaderName();

  public:
    RequestData() : state(Start){};
    std::string getMethod() const;
    std::string getRequestTarget() const;
    std::string getPath() const;
    std::string getQuery() const;
    std::string getVersion() const;
    Header getHeader() const;
    std::string getBody() const;
    std::string getServerPort() const;
    std::string getServerIP() const;
    std::string getClientPort() const;
    std::string getClientIP() const;
    long long getClientMaxBodySize() const;
    void setClientData(sockaddr_in client);
    void setServerData(sockaddr_in host);

    // TEMPORARY
    void appendData(const std::string& data);
    void clearData();
    void clearHeaderData();
    bool isHeaderComplete();
    bool isBodyCompleteByContentLength();
    bool isBodyCompleteByTransferEncoding();

    friend class Request;
};
