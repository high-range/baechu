#pragma once

#include <string>

#include "RequestData.hpp"
#include "Response.hpp"

class Worker {
  public:
    ResponseData handleRequest(const RequestData& request);

  private:
    std::map<std::string, std::string> header;  // Response header

    bool isStaticRequest(const RequestData& request);
    ResponseData handleStaticRequest(const RequestData& request);
    ResponseData handleDynamicRequest(const RequestData& request);
    ResponseData doGet(const RequestData& request);
    ResponseData doPost(const RequestData& request);
    ResponseData doDelete(const RequestData& request);
    void fetchHeaders(const RequestData& request);
    std::string getFullPath(const std::string& host, const std::string& path);
};

// Utility functions used in Worker.cpp
bool isFile(const std::string& fullPath);
std::string doGetFile(const std::string& fullPath);
std::string getFileLink(const std::string& hostname, int port,
                        const std::string& dirname,
                        const std::string& filename);
std::string generateHTML(const std::string& fullPath, const std::string& host,
                         int port, const std::string& path);
std::string doGetDirectory(const std::string& fullPath, const std::string& host,
                           const std::string& path);
std::string generateFilename();
bool saveFile(const std::string& dir, const std::string& content);
