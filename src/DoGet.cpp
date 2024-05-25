#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

typedef std::map<std::string, std::string> attributes;

class ServerConfig {
  public:
    std::string listen;
    std::string serverName;
    std::string root;
    std::map<std::string, attributes> locations;
};

class Configuration {
  public:
    std::vector<ServerConfig> servers;
};

const std::string cgiAttr = "cgi";

ServerConfig makeServerConfig() {
    ServerConfig serverConfig;

    serverConfig.listen = "8080";
    serverConfig.serverName = "localhost";
    serverConfig.root = "/Users/enihsgnir/workspace/baechu/src/";
    serverConfig.locations["/"].insert(std::make_pair("index", "index.html"));
    serverConfig.locations["/cgi-bin"].insert(std::make_pair("cgi", "true"));

    return serverConfig;
}

Configuration makeConfig() {
    Configuration config;
    config.servers.push_back(makeServerConfig());
    return config;
}

Configuration config = makeConfig();

ServerConfig getServerConfig(const std::string& host) {
    std::vector<ServerConfig> servers = config.servers;
    for (std::vector<ServerConfig>::iterator it = servers.begin();
         it != servers.end(); it++) {
        if (it->serverName == host) {
            return *it;
        }
    }

    throw "server not found";
}

std::string getFullPath(const std::string& host, const std::string& path) {
    ServerConfig serverConfig = getServerConfig(host);
    return serverConfig.root + path;
}

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.find(prefix) == 0;
}

template <typename T>
bool contains(std::vector<T> vec, T elem) {
    return std::find(vec.begin(), vec.end(), elem) != vec.end();
}

template <typename K, typename V>
bool contains(std::map<K, V> map, K key) {
    return map.find(key) != map.end();
}

bool isStaticRequest(const std::string& host, const std::string& path) {
    ServerConfig serverConfig = getServerConfig(host);

    std::map<std::string, attributes> locations = serverConfig.locations;
    for (std::map<std::string, attributes>::iterator it = locations.begin();
         it != locations.end(); it++) {
        if (startsWith(path, it->first)) {
            return !contains(it->second, cgiAttr);
        }
    }

    throw "path not found";
}

bool isFile(const std::string& fullPath) {
    struct stat statBuffer;
    if (stat(fullPath.c_str(), &statBuffer) != 0) {
        throw "stat failed";
    }

    if (S_ISREG(statBuffer.st_mode)) {
        return true;
    } else if (S_ISDIR(statBuffer.st_mode)) {
        return false;
    }
    throw "unknown file type";
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

std::string getFileLink(const std::string hostname, int port,
                        const std::string& dirname,
                        const std::string& filename) {
    std::stringstream ss;

    ss << "<p><a href=\"";
    ss << "http://" << hostname << ":" << port;
    ss << dirname << filename << "\">";
    ss << filename;
    ss << "</a></p>\n";

    return ss.str();
}

std::string generateHTML(const std::string& fullPath, const std::string& host,
                         int port, const std::string& path) {
    DIR* dir = opendir(fullPath.c_str());
    if (dir == NULL) {
        throw "could not open directory";
    }

    std::stringstream ss;

    ss << "<!DOCTYPE html>\n<html>\n<head>\n";
    ss << "<title>Index of " << path << "</title>\n";
    ss << "</head>\n<body>\n";
    ss << "<h1>Index of " << path << "</h1>\n";

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

std::string doGet(const std::string& host, const std::string& path) {
    std::string fullPath = getFullPath(host, path);

    if (isFile(fullPath)) {
        return doGetFile(fullPath);
    }
    return doGetDirectory(fullPath, host, path);
}

int main() {
    try {
        std::string file = doGet("localhost", "/");
        std::cout << file;
    } catch (const char* e) {
        std::cerr << e << std::endl;
    }

    return 0;
}
