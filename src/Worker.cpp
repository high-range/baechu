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

// FOR DEVELOPMENT

static std::string _rootDirectory = std::string(getenv("PWD"));

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

    host = header[HOST_HEADER];

    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        domain = host.substr(0, colonPos);
        port = host.substr(colonPos + 1);
    } else {
        domain = host;
        port = "80";
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
    Configuration& config = Configuration::getInstance();

    std::string rootDirectory = config.getRootDirectory(domain, port, path);

    // TODO : getRootDirectory should be changed to longest prefix matching
    // (exact matching -> longest prefix matching)
    if (rootDirectory.empty()) {
        rootDirectory += "test/static";
    }
    // else if (!rootDirectory.empty() && rootDirectory.back() != '/') {
    //     rootDirectory += '/';
    // }

    // Construct the full path
    std::string fullPath = rootDirectory + path;

    // Normalize the path to avoid issues with double slashes
    // for (std::string::size_type pos = fullPath.find("//");
    //      pos != std::string::npos; pos = fullPath.find("//")) {
    //     fullPath.erase(pos, 1);
    // }
    // std::cout << "Root directory: " << rootDirectory << std::endl;
    // std::cout << "Full path: " << fullPath << std::endl;
    return fullPath;
}

ResponseData Worker::handleStaticRequest(const RequestData& request) {
    const std::string& method = request.getMethod();

    if (host.empty()) {
        return ResponseData(400);
    } else if (method == GET) {
        return doGet(request);
    } else if (method == POST) {
        return doPost(request);
    } else if (method == DELETE) {
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

ResponseData Worker::doGetFile() {
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        throw "file not found";
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
        throw "could not open directory";
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

ResponseData Worker::doGet(const RequestData& request) {
    try {
        if (isFile(fullPath)) {
            return doGetFile();
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        // 예외가 발생하면 404 Not Found 응답을 반환
        return ResponseData(404, "Not Found: " + std::string(e.what()));
    }

    return doGetDirectory();
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
    std::string content = request.getBody();

    if (saveFile(fullPath, content)) {
        return ResponseData(201, content);  // Return 201 Created if successful
    }
    return ResponseData(500);
}

ResponseData Worker::doDelete(const RequestData& request) {
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
    envMap["REMOTE_ADDR"] = "";  // TODO: client address
    envMap["REMOTE_HOST"] = "";
    envMap["REMOTE_IDENT"] = "";
    envMap["REMOTE_USER"] = "";
    envMap["REQUEST_METHOD"] = request.getMethod();
    envMap["SCRIPT_NAME"] = scriptName;
    envMap["SERVER_NAME"] = "";  // TODO: Configuration::Block::name
    envMap["SERVER_PORT"] = port;
    envMap["SERVER_PROTOCOL"] = VERSION;
    envMap["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
    return envMap;
}

ResponseData Worker::handleRequest() {
    if (isStatic) {
        return handleStaticRequest(request);
    }
    return handleDynamicRequest();
}
