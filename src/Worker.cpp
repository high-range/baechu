#include "Worker.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Configuration.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Worker.hpp"

// FOR DEVELOPMENT

static std::string _rootDirectory = std::string(getenv("PWD"));

static const char* _cgiDirectoriesArray[] = {"/test/cgi-bin"};
static std::vector<std::string> _cgiDirectories(
    _cgiDirectoriesArray,
    _cgiDirectoriesArray +
        sizeof(_cgiDirectoriesArray) / sizeof(_cgiDirectoriesArray[0]));

static bool _isDynamicRequest(const std::string& path) {
    for (std::vector<std::string>::iterator it = _cgiDirectories.begin();
         it != _cgiDirectories.end(); it++) {
        if (path.find(*it) == 0) {
            return true;
        }
    }
    return false;
}

// END FOR DEVELOPMENT

Worker::Worker(const RequestData& request) : request(request) {
    header = request.getHeader();
}

std::string Worker::getFullPath(const std::string& host,
                                const std::string& path) {
    (void)host;
    // Configuration config(host);
    // std::string rootDirectory = config.getRootDirectory();
    return _rootDirectory + path;
}

bool Worker::isStaticRequest(const RequestData& request) {
    if (_isDynamicRequest(request.getPath())) {
        return false;
    }

    const std::string& method = request.getMethod();
    return (method == "GET" || method == "POST" || method == "DELETE");
}

ResponseData Worker::handleStaticRequest(const RequestData& request) {
    const std::string& method = request.getMethod();

    if (header.find("Host") == header.end()) {
        return ResponseData(400);
    } else if (method == "GET") {
        return doGet(request);
    } else if (method == "POST") {
        return doPost(request);
    } else if (method == "DELETE") {
        return doDelete(request);
    }
    return ResponseData(405);
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

ResponseData Worker::doGet(const RequestData& request) {
    std::string host = header["Host"];
    std::string fullPath = getFullPath(host, request.getPath());

    try {
        if (isFile(fullPath)) {
            return ResponseData(200, doGetFile(fullPath));
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        // 예외가 발생하면 404 Not Found 응답을 반환
        return ResponseData(404, "Not Found: " + std::string(e.what()));
    }

    return ResponseData(200, doGetDirectory(fullPath, host, request.getPath()));
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

ResponseData Worker::doPost(const RequestData& request) {
    std::string host = header["Host"];
    std::string fullPath = getFullPath(host, request.getPath());
    std::string content = request.getBody();

    if (saveFile(fullPath, content)) {
        return ResponseData(201, content);  // Return 201 Created if successful
    }
    return ResponseData(500);
}

ResponseData Worker::doDelete(const RequestData& request) {
    std::string host = header["Host"];
    std::string fullPath = getFullPath(host, request.getPath());

    struct stat buffer;
    if (stat(fullPath.c_str(), &buffer) == 0) {
        if (std::remove(fullPath.c_str()) == 0) {
            return ResponseData(204);  // Return 204 No Content if successful
        }
        return ResponseData(500);
    }
    return ResponseData(404);
}

ResponseData Worker::handleDynamicRequest() {
    std::istringstream response(runCgi());

    Headers headers;
    for (std::string line; std::getline(response, line);) {
        if (line.empty()) {
            break;
        }

        size_t colonPos = line.find(':');
        std::string key = line.substr(0, colonPos);

        if (line[colonPos + 1] == ' ') {
            headers[key] = line.substr(colonPos + 2);
        } else {
            headers[key] = line.substr(colonPos + 1);
        }
    }

    int statusCode = 200;
    if (headers.find("Status") != headers.end()) {
        statusCode = std::atoi(headers["Status"].c_str());
        headers.erase("Status");
    }

    std::string body;
    std::getline(response, body, '\0');

    return ResponseData(statusCode, headers, body);
}

char** makeEnvp(CgiEnvMap& envMap) {
    char** envp = new char*[envMap.size() + 1];

    int i = 0;
    for (CgiEnvMap::iterator it = envMap.begin(); it != envMap.end(); it++) {
        std::string env = it->first + "=" + it->second;
        envp[i] = new char[env.size() + 1];
        std::strcpy(envp[i], env.c_str());
        i++;
    }
    envp[i] = NULL;

    return envp;
}

std::string Worker::runCgi() {
    int fds[2];
    pipe(fds);

    pid_t pid = fork();
    if (pid == 0) {
        close(fds[0]);

        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);

        std::string fullPath = getFullPath(header["Host"], request.getPath());

        CgiEnvMap envMap = createCgiEnvMap();
        char** envp = makeEnvp(envMap);

        execve(fullPath.c_str(), NULL, envp);

        exit(EXIT_FAILURE);
    }

    close(fds[1]);

    std::ostringstream ss;
    char buf[4096];
    ssize_t n;
    while ((n = read(fds[0], buf, sizeof(buf))) > 0) {
        ss.write(buf, n);
    }

    close(fds[0]);
    waitpid(pid, NULL, 0);
    return ss.str();
}

CgiEnvMap Worker::createCgiEnvMap() {
    CgiEnvMap envMap;
    envMap["AUTH_TYPE"] = "";
    envMap["CONTENT_LENGTH"] = header["Content-Length"];
    envMap["CONTENT_TYPE"] = header["Content-Type"];
    envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
    envMap["PATH_INFO"] = "";
    envMap["PATH_TRANSLATED"] = "";
    envMap["QUERY_STRING"] = "";
    envMap["REMOTE_ADDR"] = "";
    envMap["REMOTE_HOST"] = "";
    envMap["REMOTE_IDENT"] = "";
    envMap["REMOTE_USER"] = "";
    envMap["REQUEST_METHOD"] = "";
    envMap["SCRIPT_NAME"] = "";
    envMap["SERVER_NAME"] = "";
    envMap["SERVER_PORT"] = "";
    envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
    envMap["SERVER_SOFTWARE"] = "baechu/0.1";
    return envMap;
}

ResponseData Worker::handleRequest() {
    if (isStaticRequest(request)) {
        return handleStaticRequest(request);
    }
    return handleDynamicRequest();
}
