#include "Worker.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Configuration.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Worker.hpp"

std::string Worker::getFullPath(const std::string& host,
                                const std::string& path) {
    // Configuration config(host);
    // std::string rootDirectory = config.getRootDirectory();
    std::string rootDirectory = "/Users/leesiha/42/nginxTest";
    return rootDirectory + path;
}

bool Worker::isStaticRequest(const RequestData& request) {
    const std::string& method = request.getMethod();
    return (method == "GET" || method == "POST" || method == "DELETE");
}

void Worker::handleStaticRequest(const RequestData& request) {
    const std::string& method = request.getMethod();
    std::pair<int, std::string> response;

    if (header.find("Host") == header.end()) {
        response = std::make_pair(400, "");
    } else {
        if (method == "GET") {
            response = doGet(request);
        } else if (method == "POST") {
            response = doPost(request);
        } else if (method == "DELETE") {
            response = doDelete(request);
        } else {
            response = std::make_pair(405, "");
        }
    }

    // Clear the vector and add the new response
    value.clear();
    value.push_back(response);
}

void Worker::handleDynamicRequest(const RequestData& request) {
    value.push_back(std::make_pair(501, "Not Implemented"));
    return;
}

bool isFile(const std::string& fullPath) {
    struct stat statBuffer;
    if (stat(fullPath.c_str(), &statBuffer) != 0) {
        throw std::runtime_error(
            "stat failed for path: " + fullPath +
            " with error: " + std::string(strerror(errno)));
    }

    if (S_ISREG(statBuffer.st_mode)) {
        return true;  // 일반 파일인 경우 true 반환
    } else if (S_ISDIR(statBuffer.st_mode)) {
        return false;  // 디렉토리인 경우 false 반환
    }
    throw std::runtime_error(
        "unknown file type: " +
        fullPath);  // 알 수 없는 파일 유형인 경우 예외 발생
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

std::pair<int, std::string> Worker::doGet(const RequestData& request) {
    std::string host = header.at("Host").front();
    std::string fullPath = getFullPath(host, request.getPath());

    try {
        if (isFile(fullPath)) {
            return std::make_pair(200, doGetFile(fullPath));
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        // 예외가 발생하면 404 Not Found 응답을 반환
        return std::make_pair(404, "Not Found: " + std::string(e.what()));
    }

    return std::make_pair(200,
                          doGetDirectory(fullPath, host, request.getPath()));
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

std::pair<int, std::string> Worker::doPost(const RequestData& request) {
    std::string host = header.at("Host").front();
    std::string fullPath = getFullPath(host, request.getPath());
    std::string content = request.getBody();

    if (saveFile(fullPath, content)) {
        return std::make_pair(201,
                              content);  // Return 201 Created if successful
    } else {
        return std::make_pair(500, "");
    }
}

std::pair<int, std::string> Worker::doDelete(const RequestData& request) {
    std::string host = header.at("Host").front();
    std::string fullPath = getFullPath(host, request.getPath());

    struct stat buffer;
    if (stat(fullPath.c_str(), &buffer) == 0) {
        if (std::remove(fullPath.c_str()) == 0) {
            return std::make_pair(204,
                                  "");  // Return 204 No Content if successful
        } else {
            return std::make_pair(500, "");
        }
    } else {
        return std::make_pair(404, "");
    }
}

void Worker::fetchHeaders(const RequestData& request) {
    header = request.getHeader();
}

std::vector<std::pair<int, std::string> > Worker::handleRequest(
    const RequestData& request) {
    fetchHeaders(request);
    if (isStaticRequest(request)) {
        handleStaticRequest(request);
        return value;
    } else {
        handleDynamicRequest(request);
        return value;
    }
}
