#include "Worker.hpp"

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Configuration.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Utils.hpp"

volatile sig_atomic_t timeout_occurred = 0;

void timeoutHandler(int signum) { 
    (void)signum;
    timeout_occurred = 1; 
}

void Worker::setPath(const std::string& path) {
    this->path = path;
    location = path.substr(0, path.rfind('/') + 1);
    fullPath = getFullPath(path);
}

Worker::Worker(const RequestData& request) : request(request) {
    ip = request.getServerIP();
    port = request.getServerPort();
    serverName = getServerName(request.getHeader()[HOST_HEADER]);

    method = request.getMethod();
    setPath(request.getPath());

    isStatic = true;
}

std::string Worker::getFullPath(const std::string& path) {
    Configuration& config = Configuration::getInstance();
    std::string root = config.getRootDirectory(ip, port, serverName, location);
    if (*(root.end() - 1) == '/') {
        root.pop_back();
    }
    return root + path;
}

ResponseData Worker::handleStaticRequest() {
    Configuration& config = Configuration::getInstance();

    if (config.isLocationHaveRedirect(ip, port, serverName, location)) {
        std::vector<std::string> redirectionInfo =
            config.getRedirectionInfo(ip, port, serverName, location);
        std::string statusCode = redirectionInfo[0];
        std::string redirectPath = redirectionInfo[1];
        Headers headers;
        headers["Location"] = redirectPath;
        return ResponseData(stringToInteger(statusCode), headers);
    }
    if (!config.isMethodAllowedFor(ip, port, serverName, location, method)) {
        return ResponseData(405);
    }

    if (method == GET) {
        return doGet();
    } else if (method == POST) {
        return doPost();
    } else if (method == DELETE) {
        return doDelete();
    }
    return ResponseData(405);
}

ResponseData Worker::doGetFile() {
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        return ResponseData(403);
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    std::string ext = fullPath.substr(fullPath.rfind(".") + 1);
    ext = lower(ext);

    std::string type;
    if (ext == "html" || ext == "htm") {
        type = "text/html; charset=utf-8";
    } else if (ext == "css") {
        type = "text/css; charset=utf-8";
    } else if (ext == "js") {
        type = "application/javascript";
    } else if (ext == "txt") {
        type = "text/plain; charset=utf-8";
    } else if (ext == "json") {
        type = "application/json";
    } else if (ext == "xml") {
        type = "text/xml; charset=utf-8";
    } else if (ext == "jpeg" || ext == "jpg") {
        type = "image/jpeg";
    } else if (ext == "png") {
        type = "image/png";
    } else if (ext == "gif") {
        type = "image/gif";
    } else if (ext == "svg") {
        type = "image/svg+xml";
    } else if (ext == "pdf") {
        type = "application/pdf";
    } else {
        type = "application/octet-stream";
    }

    Headers headers;
    headers[CONTENT_TYPE_HEADER] = type;

    return ResponseData(200, headers, ss.str());
}

ResponseData Worker::doGetDirectory() {
    DIR* dir = opendir(fullPath.c_str());
    if (dir == NULL) {
        return ResponseData(403);
    }

    std::ostringstream ss;
    ss << "<html>" << "\r\n";
    ss << "<head><title>Index of " << path << "</title></head>" << "\r\n";
    ss << "<body>" << "\r\n";
    ss << "<h1>Index of " << path << "</h1>";

    ss << "<hr><pre>";
    ss << "<a href=\"../\">../</a>" << "\r\n";
    for (struct dirent* entry = readdir(dir); entry; entry = readdir(dir)) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        if (entry->d_type == DT_DIR) {
            name += "/";
        }

        ss << "<a href=\"" << name << "\">" << name << "</a>" << "\r\n";
    }
    ss << "</pre><hr>";

    ss << "</body>" << "\r\n";
    ss << "</html>" << "\r\n";

    closedir(dir);

    Headers headers;
    headers[CONTENT_TYPE_HEADER] = "text/html; charset=utf-8";

    return ResponseData(200, headers, ss.str());
}

ResponseData Worker::doGet() {
    Configuration& config = Configuration::getInstance();

    struct stat buf;
    if (stat(fullPath.c_str(), &buf) != 0) {
        std::cerr << "File not found" << std::endl;
        return ResponseData(404);
    }

    if (*(path.end() - 1) != '/') {
        if (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            return doGetFile();
        } else if (S_ISDIR(buf.st_mode)) {
            Headers headers;
            headers["Location"] = path + "/";
            return ResponseData(301, headers);
        }
        std::cerr << "Unexpected file type" << std::endl;
        return ResponseData(404);
    }

    std::vector<std::string> indexes =
        config.getIndexList(ip, port, serverName, location);
    for (std::vector<std::string>::iterator it = indexes.begin();
         it != indexes.end(); it++) {
        std::string indexPath = fullPath + *it;
        if (stat(indexPath.c_str(), &buf) == 0) {
            fullPath = indexPath;
            return doGetFile();
        }
    }

    if (config.isDirectoryListingEnabled(ip, port, serverName, location)) {
        return doGetDirectory();
    }

    return ResponseData(403);
}

ResponseData Worker::doPost() {
    struct stat buf;
    if (stat(fullPath.c_str(), &buf) == 0) {
        if (S_ISDIR(buf.st_mode)) {
            return ResponseData(405);
        }
        if (S_ISREG(buf.st_mode)) {
            // 디렉토리의 write 권한 확인 (파일 수정 권한 확인)
            std::string dirPath = fullPath.substr(0, fullPath.rfind('/'));
            if (access(dirPath.c_str(), W_OK) != 0) {
                return ResponseData(403);
            }
            // modify the file
            std::ofstream file(fullPath, std::ios::trunc);
            if (file.is_open()) {
                file << request.getBody();
                file.close();
                return ResponseData(200);
            }
        }
    }
    // create a new file
    std::ofstream file(fullPath, std::ios::out);
    if (file.is_open()) {
        file << request.getBody();
        file.close();
        return ResponseData(201);
    }
    std::cerr << "Failed to create a file" << std::endl;
    return ResponseData(500);
}

ResponseData Worker::doDelete() {
    struct stat buf;
    if (stat(fullPath.c_str(), &buf) == 0) {
        if (S_ISDIR(buf.st_mode)) {
            return ResponseData(405);  // 디렉토리 삭제는 허용하지 않음
        } else if (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            // 파일이 속한 디렉토리의 쓰기 권한 확인
            std::string dirPath = fullPath.substr(0, fullPath.rfind('/'));
            if (access(dirPath.c_str(), W_OK) != 0) {
                return ResponseData(403);  // 디렉토리에 대한 쓰기 권한 없음
            }
            // 파일 삭제
            if (std::remove(fullPath.c_str()) == 0) {
                return ResponseData(200);  // 성공적으로 삭제됨
            }
            return ResponseData(500);  // 파일 삭제 실패
        }
        return ResponseData(404);  // 파일이 아니거나 디렉토리도 아님
    }
    return ResponseData(404);  // 파일이 존재하지 않음
}

ResponseData Worker::handleDynamicRequest() {
    struct stat buf;
    if (stat(fullPath.c_str(), &buf) != 0) {
        std::cerr << "File not found" << std::endl;
        return ResponseData(403);
    } else if (!S_ISREG(buf.st_mode) && !S_ISLNK(buf.st_mode)) {
        std::cerr << "Not a regular file" << std::endl;
        return ResponseData(403);
    } else if (access(fullPath.c_str(), X_OK) != 0) {
        std::cerr << "No permission to execute" << std::endl;
        return ResponseData(403);
    }

    std::string response = runCgi();
    if (response == "504 Gateway Time-out") {
        return ResponseData(504);
    } else if (response == "Internal Server Error") {
        return ResponseData(500);
    }
    std::istringstream ss(response);

    Headers headers;
    for (std::string line; std::getline(ss, line);) {
        if (line.empty()) {
            break;
        }

        if (*(line.end() - 1) == '\r') {
            line.pop_back();
        }

        size_t colonPos = line.find(':');
        std::string key = lower(line.substr(0, colonPos));

        if (line[colonPos + 1] == ' ') {
            headers[key] = line.substr(colonPos + 2);
        } else {
            headers[key] = line.substr(colonPos + 1);
        }
    }

    int statusCode = 200;
    std::string reasonPhrase;
    if (headers.find("status") != headers.end()) {
        std::istringstream ss(headers["status"]);

        ss >> statusCode;

        std::string s;
        while (ss >> s) {
            reasonPhrase += " " + s;  // include leading space
        }

        headers.erase("status");
    }

    std::string body;
    std::getline(ss, body, '\0');

    return ResponseData(statusCode, headers, body)
        .withReasonPhrase(reasonPhrase);
}

bool Worker::setupPipes(int fds[2], int inFds[2], int errFds[2]) {
    if (pipe(fds) == -1 || pipe(inFds) == -1 || pipe(errFds) == -1) {
        std::cerr << "pipe failed" << std::endl;
        return false;
    }
    return true;
}

pid_t Worker::forkAndSetupChild(int fds[2], int inFds[2], int errFds[2]) {
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "fork failed" << std::endl;
        close(fds[0]);
        close(fds[1]);
        close(inFds[0]);
        close(inFds[1]);
        close(errFds[0]);
        close(errFds[1]);
        return -1;
    }

    if (pid == 0) {  // child process
        close(fds[0]);
        close(inFds[1]);
        close(errFds[0]);

        if (dup2(fds[1], STDOUT_FILENO) == -1) {
            std::cerr << "dup2 failed for stdout" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (dup2(inFds[0], STDIN_FILENO) == -1) {
            std::cerr << "dup2 failed for stdin" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (dup2(errFds[1], STDERR_FILENO) == -1) {
            std::cerr << "dup2 failed for stderr" << std::endl;
            exit(EXIT_FAILURE);
        }

        close(fds[1]);
        close(inFds[0]);
        close(errFds[1]);

        CgiEnvMap envMap = createCgiEnvMap();
        char** envp = makeEnvp(envMap);
        char** args = makeArgs(exePath, fullPath, pathInfo);

        if (!isExecutable(exePath)) {
            std::cerr << "Not an executable file" << std::endl;
            exit(EXIT_FAILURE);
        }

        execve(exePath.c_str(), args, envp);
        std::cerr << "execve failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    return pid;
}

void Worker::writeRequestBody(int inFd) {
    if (write(inFd, request.getBody().c_str(), request.getBody().size()) == -1) {
        std::cerr << "write failed" << std::endl;
    }
    close(inFd);
}

void Worker::setupSignalHandler() {
    struct sigaction sa;
    sa.sa_handler = timeoutHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
}

std::string Worker::readFromChild(int fd) {
    std::ostringstream ss;
    char buf[4096];
    ssize_t n;

    while (true) {
        n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            ss.write(buf, n);
        } else if (n == -1) {
            if (timeout_occurred) {
                std::cerr << "read failed: Timeout occurred" << std::endl;
                close(fd);
                return "504 Gateway Time-out";
            } else {
                std::cerr << "read failed: Internal error" << std::endl;
                close(fd);
                return "Internal Server Error";
            }
        } else {
            break;
        }
    }

    close(fd);
    return ss.str();
}

std::string Worker::handleTimeout(pid_t pid, int status) {
    pid_t result = waitpid(pid, &status, WNOHANG);
    if (result == 0) {
        if (timeout_occurred) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            alarm(0);  // 알람 취소
            return "504 Gateway Time-out";
        }
    } else if (result == -1) {
        std::cerr << "waitpid failed" << std::endl;
        return "Internal Server Error";
    }
    alarm(0);  // 알람 취소
    return "";
}

std::string Worker::runCgi() {
    int fds[2], inFds[2], errFds[2];
    if (!setupPipes(fds, inFds, errFds)) {
        return "Internal Server Error";
    }

    pid_t pid = forkAndSetupChild(fds, inFds, errFds);
    if (pid == -1) {
        return "Internal Server Error";
    }

    close(fds[1]);
    close(inFds[0]);
    close(errFds[1]);

    writeRequestBody(inFds[1]);
    setupSignalHandler();

    timeout_occurred = 0;
    alarm(5);  // 5초 후에 타임아웃 발생

    std::string result = readFromChild(fds[0]);
    std::string timeoutResult = handleTimeout(pid, 0);
    if (!timeoutResult.empty()) {
        return timeoutResult;
    }
    std::string stderrOutput = readFromChild(errFds[0]);
    if (!stderrOutput.empty()) {
        return "Internal Server Error";
    }

    return result;
}

CgiEnvMap Worker::createCgiEnvMap() {
    CgiEnvMap envMap;
    envMap["AUTH_TYPE"] = "";
    envMap["CONTENT_LENGTH"] = request.getHeader()[CONTENT_LENGTH_HEADER];
    envMap["CONTENT_TYPE"] = request.getHeader()[CONTENT_TYPE_HEADER];
    envMap["GATEWAY_INTERFACE"] = GATEWAY_INTERFACE;
    envMap["PATH_INFO"] = pathInfo;
    envMap["PATH_TRANSLATED"] = getFullPath("/") + pathInfo;
    envMap["QUERY_STRING"] = request.getQuery();
    envMap["REMOTE_ADDR"] = request.getClientIP();
    envMap["REMOTE_HOST"] = "";
    envMap["REMOTE_IDENT"] = "";
    envMap["REMOTE_USER"] = "";
    envMap["REQUEST_METHOD"] = method;
    envMap["SCRIPT_NAME"] = scriptName;
    envMap["SERVER_NAME"] = request.getServerIP();
    envMap["SERVER_PORT"] = request.getServerPort();
    envMap["SERVER_PROTOCOL"] = VERSION;
    envMap["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
    return envMap;
}

ResponseData Worker::handleRequest() {
    size_t dotPos = path.rfind('.');

    if (dotPos != std::string::npos) {
        size_t pathEndPos = determinePathEndPos(dotPos);

        std::string ext = lower(path.substr(dotPos, pathEndPos - dotPos));

        if (isCgiExtension(ext)) {
            configureCgiRequest(ext, pathEndPos);
        }
    }

    if (isStatic) {
        return handleStaticRequest();
    }
    return handleDynamicRequest();
}

size_t Worker::determinePathEndPos(size_t dotPos) {
    size_t pathEndPos = path.find('/', dotPos);
    if (pathEndPos == std::string::npos) {
        pathEndPos = path.length();
    }
    return pathEndPos;
}

bool Worker::isCgiExtension(const std::string& ext) {
    Configuration& config = Configuration::getInstance();
    std::vector<std::string> cgiExtensions = config.getCgiExtensions(ip, port, serverName);
    for (std::vector<std::string>::const_iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); ++it) {
        if (ext == *it) {
            isStatic = false;
            return true;
        }
    }
    return false;
}

void Worker::configureCgiRequest(const std::string& ext, size_t pathEndPos) {
    Configuration& config = Configuration::getInstance();

    if (pathEndPos == path.length()) {
        pathInfo = "";
    } else {
        pathInfo = path.substr(pathEndPos);
    }
    
    scriptName = path.substr(0, pathEndPos);
    std::string cgiPath = config.getCgiPath(ip, port, serverName, ext);
    fullPath = cgiPath + scriptName;
    exePath = config.getInterpreterPath(ip, port, serverName, ext);
}

ResponseData Worker::resolveErrorPage(ResponseData& response) {
    Configuration& config = Configuration::getInstance();

    std::string errorPage = config.getErrorPageFromServer(
        ip, port, serverName, to_string(response.statusCode));

    if (!errorPage.empty()) {
        std::string errorPagePath = getFullPath(errorPage);
        std::string errorPageContent =
            loadErrorPage(response.statusCode, errorPagePath);
        if (errorPageContent.empty()) return ResponseData(500);
        return ResponseData(response.statusCode, errorPageContent);
    } else if (response.statusCode >= 400 && response.statusCode <= 599) {
        std::string errorPageContent = loadErrorPage(response.statusCode, "");
        if (errorPageContent.empty()) return ResponseData(500);
        return ResponseData(response.statusCode, errorPageContent);
    }

    return response;
}
