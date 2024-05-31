#include "Worker.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Configuration.hpp"
#include "Request.hpp"
#include "Response.hpp"

std::string Worker::getFullPath(const std::string& host,
                                const std::string& path) {
    // Configuration config(host);
    // std::string rootDirectory = config.getRootDirectory();
    std::string rootDirectory = "/Users/leesiha/42/nginxTest";
    return rootDirectory + path;
}

bool Worker::isStaticRequest(const Request& request) {
    const std::string& method = request.getMethod();
    return (method == "GET" || method == "POST" || method == "DELETE");
}

Response Worker::handleStaticRequest(const Request& request) {
    const std::string& method = request.getMethod();
    std::string body;

    try {
        if (method == "GET") {
            body = doGet(request);
        } else if (method == "POST") {
            body = doPost(request);
        } else if (method == "DELETE") {
            body = doDelete(request);
        } else {
            return Response(405, "Method Not Allowed");
        }
        return Response(200, body);
    } catch (const char* err) {
        return Response(500, err);
    }
}

Response Worker::handleDynamicRequest(const Request& request) {
    return Response(501, "Not Implemented");
}

bool isFile(const std::string& fullPath) {
    struct stat statBuffer;
    if (stat(fullPath.c_str(), &statBuffer) != 0) {
        throw "stat failed";
    }

    return S_ISREG(statBuffer.st_mode);
}

std::string doGetFile(const std::string& fullPath) {
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        throw "file not found";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

std::string getFileLink(const std::string& hostname, int port,
                        const std::string& dirname,
                        const std::string& filename) {
    std::ostringstream ss;
    ss << "<p><a href=\"http://" << hostname << ":" << port << dirname
       << filename << "\">" << filename << "</a></p>\n";
    return ss.str();
}

std::string generateHTML(const std::string& fullPath, const std::string& host,
                         int port, const std::string& path) {
    DIR* dir = opendir(fullPath.c_str());
    if (dir == nullptr) {
        throw "could not open directory";
    }

    std::ostringstream ss;
    ss << "<!DOCTYPE html>\n<html>\n<head>\n<title>Index of " << path
       << "</title>\n</head>\n<body>\n<h1>Index of " << path << "</h1>\n";

    for (struct dirent* dirEntry = readdir(dir); dirEntry;
         dirEntry = readdir(dir)) {
        ss << getFileLink(host, port, path, dirEntry->d_name);
    }

    ss << "</body>\n</html>\n";
    closedir(dir);

    return ss.str();
}

std::string doGetDirectory(const std::string& fullPath, const std::string& host,
                           const std::string& path) {
    return generateHTML(fullPath, host, 8080, path);
}

std::string Worker::doGet(const Request& request) {
    std::string fullPath = getFullPath(request.getHost(), request.getUrl());
    if (isFile(fullPath)) {
        return doGetFile(fullPath);
    }
    return doGetDirectory(fullPath, request.getHost(), request.getUrl());
}

std::string generateFilename() {
    std::time_t now = std::time(0);
    int random_number = std::rand() % 9000 + 1;
    std::ostringstream filename;
    filename << "saved_file_" << now << "_" << random_number << ".html";
    // return filename.str();
    return "saved_file.html";
}

bool saveFile(const std::string& dir, const std::string& content) {
    std::string filename = generateFilename();
    std::string path = dir + "/" + filename;
    std::ofstream file(path);
    if (file.is_open()) {
        file << content;
        file.close();
        return true;
    } else {
        std::cerr << "Error opening file for writing" << std::endl;
        return false;
    }
}

std::string Worker::doPost(const Request& request) {
    std::string fullPath = getFullPath(request.getHost(), request.getUrl());
    std::string content = request.getBody();

    if (saveFile(fullPath, content)) {
        return content;  // Return 201 Created if successful
    } else {
        return "";  // Return 500 Internal Server Error
    }
}

std::string Worker::doDelete(const Request& request) {
    std::string fullPath = getFullPath(request.getHost(), request.getUrl());
    struct stat buffer;
    if (stat(fullPath.c_str(), &buffer) == 0) {
        if (std::remove(fullPath.c_str()) == 0) {
            return "";  // Return 204 No Content if successful
        } else {
            throw "DELETE failed";
        }
    } else {
        return "";  // Return 404 Not Found if the file does not exist
    }
}

Response Worker::handleRequest(const Request& request) {
    if (isStaticRequest(request)) {
        return handleStaticRequest(request);
    } else {
        return handleDynamicRequest(request);
    }
}
