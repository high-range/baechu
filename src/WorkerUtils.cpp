#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Response.hpp"
#include "Utils.hpp"
#include "Worker.hpp"

bool isUtf8(const std::string& str) {
    for (std::string::size_type i = 0; i < str.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (c >= 0x80) {
            return false;
        }
    }
    return true;
}

std::string lower(std::string s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (isupper(s[i])) {
            s[i] = tolower(s[i]);
        }
    }
    return s;
}

std::string getServerName(std::string host) {
    size_t colonPos = host.find(':');
    if (colonPos == std::string::npos) {
        return host;
    }
    return host.substr(0, colonPos);
}

char** makeArgs(const std::string& exePath, const std::string& fullPath, const std::string& pathInfo) {
    char** args = new char*[4];
    args[0] = new char[exePath.size() + 1];
    std::strcpy(args[0], exePath.c_str());
    args[1] = new char[fullPath.size() + 1];
    std::strcpy(args[1], fullPath.c_str());
    args[2] = new char[pathInfo.size() + 1];
    std::strcpy(args[2], pathInfo.c_str());
    args[3] = NULL;
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

bool isReadableFile(const std::string& path) {
    struct stat sb;
    return (stat(path.c_str(), &sb) == 0 && sb.st_mode & S_IRUSR);
}

std::string loadErrorPage(int statusCode, std::string errorPagePath) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        std::cerr << "Error: cannot get current working directory" << std::endl;
        return "";
    }
    std::string currentDir(cwd);

    bool isDefaultErrorPage = false;
    if (isReadableFile(errorPagePath) == false) {
        errorPagePath = currentDir + "/defence/default_error_page.html";
        isDefaultErrorPage = true;
    }

    std::ifstream errorFile(errorPagePath.c_str());
    if (!errorFile.is_open()) {
        std::cerr << "Error: cannot open error page file" << std::endl;
        return "";
    }

    std::ostringstream ss;
    ss << errorFile.rdbuf();
    std::string errorPageContent = ss.str();
    errorFile.close();

    if (isDefaultErrorPage) {
        std::string statusCodeStr = to_string(statusCode);
        std::string errorMessage = getReasonPhrase(statusCode);

        size_t pos = errorPageContent.find("{status_code}");
        if (pos != std::string::npos) {
            errorPageContent.replace(pos, 13, statusCodeStr);
        }
        pos = errorPageContent.find("{error_message}");
        if (pos != std::string::npos) {
            errorPageContent.replace(pos, 15, errorMessage);
        }
    }
    return errorPageContent;
}
