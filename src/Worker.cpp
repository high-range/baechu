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

static std::string lower(std::string s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (isupper(s[i])) {
            s[i] = tolower(s[i]);
        }
    }
    return s;
}

static std::string getServerName(std::string host) {
    size_t colonPos = host.find(':');
    if (colonPos == std::string::npos) {
        return host;
    }
    return host.substr(0, colonPos);
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
    if (root.back() == '/') {
        root.pop_back();
    }

    std::cout << "root: " << root << std::endl;
    std::cout << "path: " << path << std::endl;
    std::cout << "fullPath: " << root + path << std::endl;
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
        return ResponseData(std::stoi(statusCode), headers);
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
        std::cout << "File not found, fullPath: " << fullPath << std::endl;
        return ResponseData(404);
    }

    if (path.back() != '/') {
        if (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            return doGetFile();
        } else if (S_ISDIR(buf.st_mode)) {
            Headers headers;
            headers["Location"] = path + "/";
            return ResponseData(301, headers);
        }
        std::cout << "Unexpected file type" << std::endl;
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

char** makeArgs(const std::string& exePath, const std::string& scriptName) {
    char** args = new char*[3];
    args[0] = new char[exePath.size() + 1];
    std::strcpy(args[0], exePath.c_str());
    args[1] = new char[scriptName.size() + 1];
    std::strcpy(args[1], scriptName.c_str());
    args[2] = NULL;
    return args;
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

bool isExecutable(const std::string& path) {
    struct stat sb;
    return (stat(path.c_str(), &sb) == 0 && sb.st_mode & S_IXUSR);
}

std::string Worker::runCgi() {
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe");
        return "Internal Server Error";
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return "Internal Server Error";
    }

    if (pid == 0) {  // child process
        close(fds[0]);

        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);

        CgiEnvMap envMap = createCgiEnvMap();
        char** envp = makeEnvp(envMap);
        char** args = makeArgs(exePath, fullPath);

        if (!isExecutable(exePath)) {
            std::cerr << "Not an executable file" << std::endl;
            exit(EXIT_FAILURE);
        }

        execve(exePath.c_str(), args, envp);

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
    Configuration& config = Configuration::getInstance();
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        size_t dirPos = path.find('/', dotPos);
        if (dirPos == std::string::npos) {
            dirPos = path.length();
        }

        std::string ext = path.substr(dotPos, dirPos - dotPos);
        ext = lower(ext);

        std::vector<std::string> cgiExtensions =
            config.getCgiExtensions(ip, port, serverName);
        for (std::vector<std::string>::iterator it = cgiExtensions.begin();
             it != cgiExtensions.end(); it++) {
            if (ext == *it) {
                isStatic = false;
                pathInfo = path.substr(dirPos);
                scriptName = path.substr(0, dirPos);
                std::string cgiPath =
                    config.getCgiPath(ip, port, serverName, ext);
                fullPath = cgiPath + scriptName;
                exePath = config.getInterpreterPath(ip, port, serverName, ext);
                break;
            }
        }
    }

    if (isStatic) {
        return handleStaticRequest();
    }
    return handleDynamicRequest();
}

Worker Worker::redirectedTo(const std::string& path) {
    method = GET;
    setPath(path);
    isStatic = true;
    return *this;
}

ResponseData Worker::redirectOrUse(ResponseData& response) {
    Configuration& config = Configuration::getInstance();

    std::string errorPage = config.getErrorPageFromServer(
        ip, port, serverName, std::to_string(response.statusCode));

	// errorPath 설정
	char cwd[PATH_MAX];
	std::string errorPagePath;
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		// 현재 dir 설정
		std::string currentDir(cwd);
		errorPagePath = currentDir + "/defence/";
	}
	else {
		// getcwd실패 : 내부 시스템 오류 (500 error)
		// local 환경에 맞게 errorPagePath 설정해야함.
		response.statusCode = 500;
		errorPagePath = "/Users/superstar/Desktop/baechu/defence/";
	}

	// errorPage가 empty인지, 유효한 파일인지 체크
    if (!errorPage.empty()) {
		errorPagePath += errorPage;
        std::ifstream testFile(errorPagePath);
        if (testFile.is_open()) {
            testFile.close();
            return redirectedTo(errorPage).handleRequest();
        }
    }
	if (response.statusCode >= 400 && response.statusCode <= 599) {
        errorPage = "/default_error_page.html";
		errorPagePath += errorPage;

		// 에러 메시지 설정
		std::string errorMsg = getReasonPhrase(response.statusCode);

        // 상태 코드와 메시지를 작성할 파일 열기
        std::ofstream outFile(errorPagePath);
        if (outFile.is_open()) {
            outFile << "<!DOCTYPE html>\n";
            outFile << "<html lang=\"ko\">\n";
            outFile << "<head>\n";
            outFile << "    <meta charset=\"UTF-8\">\n";
            outFile << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
            outFile << "    <title>" << response.statusCode << " - Error</title>\n";
            outFile << "    <style>\n";
            outFile << "        body {\n";
            outFile << "            background-color: #f2f2f2;\n";
            outFile << "            font-family: Arial, sans-serif;\n";
            outFile << "            display: flex;\n";
            outFile << "            justify-content: center;\n";
            outFile << "            align-items: center;\n";
            outFile << "            height: 100vh;\n";
            outFile << "            margin: 0;\n";
            outFile << "        }\n";
            outFile << "        .container {\n";
            outFile << "            text-align: center;\n";
            outFile << "            background-color: #ffffff;\n";
            outFile << "            padding: 30px;\n";
            outFile << "            border-radius: 10px;\n";
            outFile << "            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);\n";
            outFile << "        }\n";
            outFile << "        h1 {\n";
            outFile << "            font-size: 96px;\n";
            outFile << "            margin: 0;\n";
            outFile << "            color: #ff6f61;\n";
            outFile << "        }\n";
            outFile << "        p {\n";
            outFile << "            font-size: 18px;\n";
            outFile << "            margin: 10px 0;\n";
            outFile << "            color: #333;\n";
            outFile << "        }\n";
            outFile << "        .btn {\n";
            outFile << "            display: inline-block;\n";
            outFile << "            margin-top: 20px;\n";
            outFile << "            padding: 10px 20px;\n";
            outFile << "            font-size: 16px;\n";
            outFile << "            color: #fff;\n";
            outFile << "            background-color: #ff6f61;\n";
            outFile << "            text-decoration: none;\n";
            outFile << "            border-radius: 5px;\n";
            outFile << "            transition: background-color 0.3s ease;\n";
            outFile << "        }\n";
            outFile << "        .btn:hover {\n";
            outFile << "            background-color: #ff3b2f;\n";
            outFile << "        }\n";
            outFile << "    </style>\n";
            outFile << "</head>\n";
            outFile << "<body>\n";
            outFile << "    <div class=\"container\">\n";
            outFile << "        <h1>" << response.statusCode << "</h1>\n";
            outFile << "        <p>" << errorMsg << "</p>\n";
            outFile << "        <a href=\"/\" class=\"btn\">홈으로 돌아가기</a>\n";
            outFile << "    </div>\n";
            outFile << "</body>\n";
            outFile << "</html>\n";
            outFile.close();
        }

        return redirectedTo(errorPage).handleRequest();
    }

    return response;
}

