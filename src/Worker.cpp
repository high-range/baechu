#include "Worker.hpp"

#include <dirent.h>
#include <stdio.h>
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

// FOR DEVELOPMENT

static std::vector<std::string> _getCgiExtensions() {
    std::vector<std::string> cgiExtensions;
    cgiExtensions.push_back(".py");
    cgiExtensions.push_back(".pl");
    return cgiExtensions;
}
static std::vector<std::string> _cgiExtensions = _getCgiExtensions();

// END FOR DEVELOPMENT

static std::string lower(std::string s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (isupper(s[i])) {
            s[i] = tolower(s[i]);
        }
    }
    return s;
}

Worker::Worker(const RequestData& request) : request(request) {
    header = request.getHeader();

    ip = request.getServerIP();
    port = request.getServerPort();

    std::string host = header[HOST_HEADER];
    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        serverName = host.substr(0, colonPos);
    } else {
        serverName = host;
    }

    path = request.getPath();
    fullPath = getFullPath(path);

    isStatic = true;

    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        size_t dirPos = path.find('/', dotPos);
        if (dirPos == std::string::npos) {
            dirPos = path.length();
        }

        std::string ext = path.substr(dotPos, dirPos - dotPos);
        ext = lower(ext);

        for (std::vector<std::string>::iterator it = _cgiExtensions.begin();
             it != _cgiExtensions.end(); it++) {
            if (ext == *it) {
                isStatic = false;
                pathInfo = path.substr(dirPos);
                scriptName = path.substr(0, dirPos);
                fullPath = getFullPath(scriptName);
                break;
            }
        }
    }
}

std::string Worker::getFullPath(const std::string& path) {
    location = path.substr(0, path.rfind('/') + 1);

    Configuration& config = Configuration::getInstance();
    std::string root = config.getRootDirectory(port, location, serverName);

    return root + path;
}

ResponseData Worker::handleStaticRequest() {
    Configuration& config = Configuration::getInstance();

    const std::string& method = request.getMethod();
    if (!config.isMethodAllowedFor(port, location, method)) {
        return ResponseData(405);
    }

    if (method == GET) {
        return doGet();
    } else if (method == POST) {
        return ResponseData(405);
    } else if (method == PUT) {
        return doPut();
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
    if (dir == nullptr) {
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
        return ResponseData(404);
    }

    if (path.back() != '/') {
        if (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            return doGetFile();
        } else if (S_ISDIR(buf.st_mode)) {
            return ResponseData(301);
        }
        return ResponseData(404);
    }

    std::vector<std::string> indexes;
    indexes.push_back("index.html");

    for (std::vector<std::string>::iterator it = indexes.begin();
         it != indexes.end(); it++) {
        std::string indexPath = fullPath + *it;
        if (stat(indexPath.c_str(), &buf) == 0) {
            fullPath = indexPath;
            return doGetFile();
        }
    }

    if (config.isDirectoryListingEnabled(port, location)) {
        return doGetDirectory();
    }

    return ResponseData(403);
}

ResponseData Worker::doPut() {
    struct stat buf;
    if (stat(fullPath.c_str(), &buf) == 0) {
        if (S_ISDIR(buf.st_mode)) {
            return ResponseData(405);
        }
        if (S_ISREG(buf.st_mode)) {
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
    return ResponseData(500);
}

ResponseData Worker::doDelete() {
    struct stat buffer;
    if (stat(fullPath.c_str(), &buffer) == 0) {
        if (remove(fullPath.c_str()) == 0) {
            return ResponseData(204);  // Return 204 No Content if successful
        }
        return ResponseData(500);
    }
    return ResponseData(404);
}

ResponseData Worker::handleDynamicRequest() {
    std::string response = runCgi();
    std::istringstream ss(response);

    Headers headers;
    for (std::string line; std::getline(ss, line);) {
        if (line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            break;
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
    if (headers.find("status") != headers.end()) {
        statusCode = std::atoi(headers["status"].c_str());
        headers.erase("status");
    }

    std::string body;
    std::getline(ss, body, '\0');

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
    envMap["CONTENT_LENGTH"] = header[CONTENT_LENGTH_HEADER];
    envMap["CONTENT_TYPE"] = header[CONTENT_TYPE_HEADER];
    envMap["GATEWAY_INTERFACE"] = GATEWAY_INTERFACE;
    envMap["PATH_INFO"] = pathInfo;
    envMap["PATH_TRANSLATED"] = "";  // TODO: rootDir + pathInfo
    envMap["QUERY_STRING"] = request.getQuery();
    envMap["REMOTE_ADDR"] = request.getClientIP();
    envMap["REMOTE_HOST"] = "";
    envMap["REMOTE_IDENT"] = "";
    envMap["REMOTE_USER"] = "";
    envMap["REQUEST_METHOD"] = request.getMethod();
    envMap["SCRIPT_NAME"] = scriptName;
    envMap["SERVER_NAME"] = "";  // TODO: Configuration::Block::name
    envMap["SERVER_PORT"] = request.getServerPort();
    envMap["SERVER_PROTOCOL"] = VERSION;
    envMap["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
    return envMap;
}

ResponseData Worker::handleRequest() {
    if (isStatic) {
        return handleStaticRequest();
    }
    return handleDynamicRequest();
}
