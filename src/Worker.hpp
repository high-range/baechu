#pragma once

#include <string>

#include "Request.hpp"
#include "Response.hpp"

class Worker {
  public:
    Response handleRequest(const Request& request);

  private:
    std::string getFullPath(const std::string& host, const std::string& path);
    bool isStaticRequest(const Request& request);
    Response handleStaticRequest(const Request& request);
    Response handleDynamicRequest(const Request& request);

    std::string doGet(const Request& request);
    std::string doPost(const Request& request);
    std::string doDelete(const Request& request);
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
