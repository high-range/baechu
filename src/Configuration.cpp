#include "Configuration.hpp"

#include <iostream>
#include <sstream>

// -------------------------- Constructor -------------------------------
Configuration* Configuration::configuration_ = NULL;
Configuration::Configuration() {}

// Static method to get the single instance of Configuration
Configuration& Configuration::getInstance() {
    if (configuration_ == NULL) configuration_ = new Configuration();
    return *configuration_;
}

// Method to initialize the Configuration with a filename
void Configuration::initialize(const std::string& filename) {
    parseConfigFile(filename);
    if (isValidServerBlockPlacement(blocks, "") == false) {
        std::cout << "Error: \"server\" directive is in the wrong location."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!isServerHavePort()) {
        exit(EXIT_FAILURE);
    }
    if (!checkMethods()) {
        exit(EXIT_FAILURE);
    }
    if (!checkErrorPage()) {
        exit(EXIT_FAILURE);
    }
    if (!isValidCgiPath()) {
        exit(EXIT_FAILURE);
    }
    if (!isDuplicatedHttp()) {
        exit(EXIT_FAILURE);
    }
}

// -------------------------- Parsing 함수 -------------------------------
void Configuration::parseConfigFile(const std::string& filename) {
    std::cout << "Set \"" << filename << "\" Setting" << std::endl;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file \"" << filename << "\""
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;

    while (std::getline(file, line)) {
        // 앞,뒤 공백 제거
        line = trim(line);

        // 빈 줄이거나, 주석이면 pass
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // 블록 시작
        if (line.find("{") != std::string::npos) {
            std::string block_name = trim(line.substr(0, line.find('{')));
            if (!isValidBlockName(block_name) ||
                !isValidKeyInBlock("main", block_name)) {
                std::cerr << "Error: Invalid block \"" << block_name
                          << "\" in \"main\"" << std::endl;
                exit(EXIT_FAILURE);
            }
            Block block;
            block.name = block_name;
            if (parseBlock(file, block)) {
                blocks.push_back(block);
            } else {
                std::cerr << "Error: Block not closed." << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            size_t space_pos = line.find(' '), tap_pos = line.find('\t');
            size_t separator_pos = std::min(space_pos, tap_pos);
            size_t end_pos = line.find(';');

            if (end_pos == std::string::npos) {
                if (end_pos == std::string::npos) {
                    std::cerr << "Error: Directive not finished with ';'"
                              << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            if (separator_pos != std::string::npos) {
                std::string key = trim(line.substr(0, separator_pos));
                if (!isValidDirectiveKey(key)) {
                    std::cerr << "Error: Unknown directive key: \"" << key
                              << "\"" << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::string value = trim(line.substr(
                    separator_pos + 1, end_pos - separator_pos - 1));
                if (value.empty()) {
                    std::cerr << "Error: There is no value for key \"" << key
                              << "\"" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (!isValidKeyInBlock("main", key)) {
                    std::cerr << "Error: directive \"" << key
                              << "\" not allowed in block \"main\""
                              << std::endl;
                    exit(EXIT_FAILURE);
                }
                simple_directives[key] = value;
            } else {
                std::cerr << "Error: No key" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

bool Configuration::parseBlock(std::ifstream& file, Block& current_block) {
    int openBraces = 1;
    std::string line;

    while (std::getline(file, line)) {
        // 앞,뒤 공백 제거
        line = trim(line);

        // 빈 줄이거나, 주석이면 pass
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line.find("{") != std::string::npos) {
            std::string block_name = trim(line.substr(0, line.find('{')));
            if (!isValidBlockName(block_name) ||
                !isValidKeyInBlock(current_block.name, block_name)) {
                std::cerr << "Error: Invalid block \"" << block_name
                          << "\" in \"" << current_block.name << "\""
                          << std::endl;
                exit(EXIT_FAILURE);
            }
            Block block;
            block.name = block_name;
            // block_name 은 '/'로 끝나야함
            if (block_name.find("location ") != std::string::npos &&
                block_name.back() != '/') {
                std::cerr << "Error: location name should be ended with \'/\'"
                          << std::endl;
                exit(EXIT_FAILURE);
            }
            if (parseBlock(file, block)) {
                current_block.sub_blocks.push_back(block);
            } else {
                std::cerr << "Error: Block not closed." << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (line.find('}') != std::string::npos) {
            openBraces--;
            if (openBraces == 0) {
                return true;
            } else {
                std::cerr << "Error: Block not closed." << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            size_t space_pos = line.find(' '), tap_pos = line.find('\t');
            size_t separator_pos = std::min(space_pos, tap_pos);
            size_t end_pos = line.find(';');

            if (end_pos == std::string::npos) {
                std::cerr << "Error: Directive not finished with ';'"
                          << std::endl;
                exit(EXIT_FAILURE);
            }
            if (separator_pos != std::string::npos) {
                std::string key = trim(line.substr(0, separator_pos));
                if (!isValidDirectiveKey(key)) {
                    std::cerr << "Error: Invalid directive key: " << key
                              << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::string value = trim(line.substr(
                    separator_pos + 1, end_pos - separator_pos - 1));
                if (value.empty()) {
                    std::cerr << "Error: There is no value for key \"" << key
                              << "\"" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (!isValidKeyInBlock(current_block.name, key)) {
                    std::cerr << "Error: directive \"" << key
                              << "\" not allowed in block \""
                              << current_block.name << "\"" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (key == "client_max_body_size") {
                    if (!std::isdigit(value[0])) {
                        std::cerr << "Error: \"client_max_body_size\" "
                                     "directive has invalid value"
                                  << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    for (size_t i = 0; i < value.length(); i++) {
                        if (!(std::isdigit(value[0]) || value[i] == 'M') ||
                            ((i < value.length() - 1) && value[i] == 'M')) {
                            std::cerr << "Error: \"client_max_body_size\" "
                                         "directive has invalid value"
                                      << std::endl;
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                if (key == "listen") {
                    if (!isValidListen(value)) {
                        std::cerr << "Error: \"listen\" directive has invalid "
                                     "value \""
                                  << value << "\"" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
                current_block.directives[key] = value;
            } else {
                std::cerr << "Error: No key" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
    if (openBraces != 0) {
        std::cerr << "Error: Block not closed." << std::endl;
        exit(EXIT_FAILURE);
    }
    return true;
}

// ---------------------------- syntax check --------------------------------
bool Configuration::hasServerBlocks(
    const std::vector<Block>& recur_block) const {
    for (std::vector<Block>::const_iterator block_it = recur_block.begin();
         block_it != recur_block.end(); ++block_it) {
        if (block_it->name == "server") {
            return true;
        }
    }
    return false;
}

bool Configuration::isValidServerBlockPlacement(
    const std::vector<Block>& recur_block,
    const std::string& upper_block) const {
    if (upper_block != "http") {
        if (hasServerBlocks(recur_block)) {
            return false;
        }
    }
    for (std::vector<Block>::const_iterator block_it = recur_block.begin();
         block_it != recur_block.end(); ++block_it) {
        if (!isValidServerBlockPlacement(block_it->sub_blocks,
                                         block_it->name)) {
            return false;
        }
    }
    return true;
}

bool Configuration::isServerHavePort() const {
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_block_it =
                     block_it->sub_blocks.begin();
                 sub_block_it != block_it->sub_blocks.end(); ++sub_block_it) {
                if (sub_block_it->name == "server") {
                    if (sub_block_it->directives.find("listen") ==
                        sub_block_it->directives.end()) {
                        std::cerr << "Error: \"server\" has not \"listen\""
                                  << std::endl;
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool Configuration::checkMethods() const {
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_it =
                     block_it->sub_blocks.begin();
                 sub_it != block_it->sub_blocks.end(); ++sub_it) {
                if (sub_it->name == "server") {
                    for (std::vector<Block>::const_iterator it =
                             sub_it->sub_blocks.begin();
                         it != sub_it->sub_blocks.end(); ++it) {
                        if (it->name.find("location ") != std::string::npos) {
                            if (it->directives.find("limit_except") !=
                                it->directives.end()) {
                                std::stringstream ss(
                                    it->directives.at("limit_except"));
                                std::string method;
                                while (ss >> method) {
                                    if (!isValidMethods(method)) {
                                        std::cerr << "Error: Invalid method: \""
                                                  << method << "\""
                                                  << std::endl;
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool Configuration::checkErrorPage() const {
    std::vector<std::string> errors;
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_it =
                     block_it->sub_blocks.begin();
                 sub_it != block_it->sub_blocks.end(); ++sub_it) {
                if (sub_it->name == "server") {
                    if (sub_it->directives.find("error_page") !=
                        sub_it->directives.end()) {
                        std::stringstream ss(
                            sub_it->directives.at("error_page"));
                        std::string word;
                        while (ss >> word) {
                            errors.push_back(word);
                        }
                        for (size_t i = 0; i < errors.size() - 1; i++) {
                            for (size_t j = 0; j < errors[i].length(); j++) {
                                if (!std::isdigit(errors[i][j])) {
                                    std::cerr << "Error: Invalid statusCode \""
                                              << errors[i] << "\"" << std::endl;
                                    return false;
                                }
                            }
                        }
                        if (errors[errors.size() - 1][0] != '/') {
                            std::cerr << "Error: Invalid path \""
                                      << errors[errors.size() - 1] << "\""
                                      << std::endl;
                            return false;
                        }
                    }
                }
                errors.clear();
            }
        }
    }
    return true;
}

bool Configuration::isValidListen(const std::string& listen_value) const {
    // Check for "ip:port", "port", or "ip"
    std::string ip;
    std::string port;
    size_t colon_pos = listen_value.find(':');

    if (colon_pos != std::string::npos) {
        // "ip:port" case
        ip = listen_value.substr(0, colon_pos);
        port = listen_value.substr(colon_pos + 1);
    } else {
        // Check if it's a port or an ip
        bool is_port = true;
        for (size_t i = 0; i < listen_value.size(); ++i) {
            if (!std::isdigit(listen_value[i])) {
                is_port = false;
                break;
            }
        }
        if (is_port) {
            // All characters are digits, it's a port
            port = listen_value;
        } else {
            // Otherwise, it's an ip
            ip = listen_value;
        }
    }

    // Validate IP if present
    if (!ip.empty()) {
        std::istringstream ip_stream(ip);
        std::string segment;
        int num_segments = 0;
        while (std::getline(ip_stream, segment, '.')) {
            if (segment.empty() || segment.length() > 3) {
                return false;
            }
            for (size_t i = 0; i < segment.size(); ++i) {
                if (!std::isdigit(segment[i])) {
                    return false;
                }
            }
            int seg_value = std::atoi(segment.c_str());
            if (seg_value < 0 || seg_value > 255) {
                return false;
            }
            ++num_segments;
        }
        if (num_segments != 4) {
            return false;
        }
    }

    // Validate port if present
    if (!port.empty()) {
        if (port.length() > 5) {
            return false;
        }
        for (size_t i = 0; i < port.size(); ++i) {
            if (!std::isdigit(port[i])) {
                return false;
            }
        }
        int port_value = std::atoi(port.c_str());
        if (port_value < 1 || port_value > 65535) {
            return false;
        }
    }

    return true;
}

bool Configuration::isValidCgiPath() const {
    std::vector<std::string> cgi;
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_it =
                     block_it->sub_blocks.begin();
                 sub_it != block_it->sub_blocks.end(); ++sub_it) {
                if (sub_it->name == "server") {
                    if (sub_it->directives.find("cgi") !=
                        sub_it->directives.end()) {
                        std::stringstream ss(sub_it->directives.at("cgi"));
                        std::string word;
                        while (ss >> word) {
                            cgi.push_back(word);
                        }
                        if (cgi.size() != 2) {
                            std::cerr << "Error: Invalid \"cgi\" format"
                                      << std::endl;
                            return false;
                        }
                        if (cgi[0][0] != '.') {
                            std::cerr << "Error: Invalid \"cgi\" extension"
                                      << std::endl;
                            return false;
                        }
                        if (cgi[1][0] != '/') {
                            std::cerr << "Error: Invalid \"cgi\" path"
                                      << std::endl;
                            return false;
                        }
                    }
                }
                cgi.clear();
            }
        }
    }
    return true;
}

bool Configuration::isDuplicatedHttp() const {
    int http_count = 0;
    for (std::vector<Block>::const_iterator it = blocks.begin();
         it != blocks.end(); ++it) {
        if (it->name == "http") {
            http_count++;
        }
    }
    if (http_count != 1) {
        std::cerr << "Error: \"http\" directive is duplicate" << std::endl;
        return false;
    }
    return true;
}

// -------------------------- 정보 가져오는 함수 ------------------------------
std::vector<std::string> Configuration::getPortNumbers() const {
    std::vector<std::string> ports;
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_it =
                     block_it->sub_blocks.begin();
                 sub_it != block_it->sub_blocks.end(); ++sub_it) {
                if (sub_it->name == "server") {
                    std::string port_number = sub_it->directives.at("listen");
                    ports.push_back(port_number);
                }
            }
        }
    }
    return ports;
}

Block Configuration::getServerBlockWithPortAndName(
    const std::string& ip, const std::string& port_number,
    const std::string& server_name) const {
    for (std::vector<Block>::const_iterator it = blocks.begin();
         it != blocks.end(); ++it) {
        if (it->name == "http") {
            // ip:port, server_name이 모두 일치하는 server 블록 탐색
            for (std::vector<Block>::const_iterator block_it =
                     it->sub_blocks.begin();
                 block_it != it->sub_blocks.end(); ++block_it) {
                const Block& block = *block_it;
                if (ip.empty()) {
                    if (block.name == "server" &&
                        block.directives.find("listen") !=
                            block.directives.end() &&
                        block.directives.at("listen") == port_number &&
                        block.directives.find("server_name") !=
                            block.directives.end() &&
                        block.directives.at("server_name") == server_name) {
                        return block;
                    }
                } else if (port_number.empty()) {
                    if (block.name == "server" &&
                        block.directives.find("listen") !=
                            block.directives.end() &&
                        block.directives.at("listen") == ip &&
                        block.directives.find("server_name") !=
                            block.directives.end() &&
                        block.directives.at("server_name") == server_name) {
                        return block;
                    }
                } else {
                    if (block.name == "server" &&
                        block.directives.find("listen") !=
                            block.directives.end() &&
                        block.directives.at("listen") ==
                            ip + ":" + port_number &&
                        block.directives.find("server_name") !=
                            block.directives.end() &&
                        block.directives.at("server_name") == server_name) {
                        return block;
                    }
                }
            }

            // port가 일치하는 server 블록 탐색
            for (std::vector<Block>::const_iterator block_it =
                     it->sub_blocks.begin();
                 block_it != it->sub_blocks.end(); ++block_it) {
                const Block& block = *block_it;
                if (block.name == "server" &&
                    block.directives.find("listen") != block.directives.end() &&
                    block.directives.at("listen") == port_number) {
                    return block;
                }
            }

            // ip가 일치하는 server 블록 탐색
            for (std::vector<Block>::const_iterator block_it =
                     it->sub_blocks.begin();
                 block_it != it->sub_blocks.end(); ++block_it) {
                const Block& block = *block_it;
                if (block.name == "server" &&
                    block.directives.find("listen") != block.directives.end() &&
                    block.directives.at("listen") == ip) {
                    return block;
                }
            }

            // server_name이 일치하는 server 블록 탐색
            for (std::vector<Block>::const_iterator block_it =
                     it->sub_blocks.begin();
                 block_it != it->sub_blocks.end(); ++block_it) {
                const Block& block = *block_it;
                if (block.name == "server" &&
                    block.directives.find("server_name") !=
                        block.directives.end() &&
                    block.directives.at("server_name") == server_name) {
                    return block;
                }
            }

            // 못 찾았을 경우, default server return
            for (std::vector<Block>::const_iterator block_it =
                     it->sub_blocks.begin();
                 block_it != it->sub_blocks.end(); ++block_it) {
                const Block& block = *block_it;
                if (block.name == "server") {
                    return block;
                }
            }
        }
    }
    return Block();
}

Block Configuration::getLongestMatchingLocation(
    const Block& server, const std::string& request_location) const {
    Block longest_matching_location;
    size_t longest_match_length = 0;

    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name.substr(0, 8) == "location") {
            std::string location_prefix = block_it->name.substr(9);

            if (request_location.empty()) {
                if (location_prefix == "/") {
                    longest_matching_location = *block_it;
                }
            }
            // 먼저 prefix가 일치하는지 체크
            if (request_location.compare(0, location_prefix.length(),
                                         location_prefix) == 0) {
                // 일치한다면, longest matching인지 체크
                if (location_prefix.length() > longest_match_length) {
                    longest_match_length = location_prefix.length();
                    longest_matching_location = *block_it;
                }
            }
        }
    }
    return longest_matching_location;
}

std::string Configuration::getRootDirectory(const std::string& ip,
                                            const std::string& port_number,
                                            const std::string& server_name,
                                            const std::string& location) const {
    // server_name과 port_number로 server Block 찾기
    Block server = getServerBlockWithPortAndName(ip, port_number, server_name);

    Block location_block = getLongestMatchingLocation(server, location);

    if (location_block.directives.find("root") !=
        location_block.directives.end()) {
        return location_block.directives.at("root");
    } else {
        if (server.directives.find("root") != server.directives.end()) {
            return server.directives.at("root");
        }
    }

    // 없을 경우 빈 문자열 return
    return "";
}

std::string Configuration::getClientMaxBodySize(
    const std::string& ip, const std::string& port,
    const std::string& server_name, const std::string& location) const {
    // port_number로 server Block 찾기
    Block server = getServerBlockWithPortAndName(ip, port, server_name);
    if (server.name.empty()) {
        return "1M";
    }

    // 가장 긴 일치 경로를 찾기 위해 모든 location 블록을 검사
    Block location_block = getLongestMatchingLocation(server, location);
    if (location_block.directives.find("client_max_body_size") !=
        location_block.directives.end()) {
        return location_block.directives.at("client_max_body_size");
    }

    // location 블록에 설정이 없으면 server 블록 설정 사용
    if (server.directives.find("client_max_body_size") !=
        server.directives.end()) {
        return server.directives.at("client_max_body_size");
    }

    // server 블록에도 설정이 없으면 http 블록 설정 사용
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            if (block_it->directives.find("client_max_body_size") !=
                block_it->directives.end()) {
                return block_it->directives.at("client_max_body_size");
            }
        }
    }

    // http 블록에도 없으면 default 1024
    return "1M";
}

std::string Configuration::getDefaultPort() const {
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_block_it =
                     block_it->sub_blocks.begin();
                 sub_block_it != block_it->sub_blocks.end(); ++sub_block_it) {
                if (sub_block_it->name == "server") {
                    std::string listen = sub_block_it->directives.at("listen");
                    if (listen.find('.') != std::string::npos &&
                        listen.find(':') != std::string::npos) {
                        size_t colon_pos = listen.find(':');
                        return listen.substr(colon_pos + 1);
                    }
                    if (listen.find('.') != std::string::npos) {
                        return "80";
                    }
                    return listen;
                }
            }
        }
    }
    return "80";  // TODO: throw
}

bool Configuration::isMethodAllowedFor(const std::string& ip,
                                       const std::string& port,
                                       const std::string& server_name,
                                       const std::string& location,
                                       const std::string& method) const {
    Block server = getServerBlockWithPortAndName(ip, port, server_name);
    Block location_block = getLongestMatchingLocation(server, location);
    if (location_block.name.empty()) {
        return false;  // TODO: throw
    }

    if (location_block.directives.find("limit_except") !=
        location_block.directives.end()) {
        return location_block.directives["limit_except"].find(method) !=
               std::string::npos;
    }
    return true;
}

std::vector<std::string> Configuration::getAllowedMethods(
    const std::string& ip, const std::string& port,
    const std::string& server_name, const std::string& location) const {
    std::vector<std::string> methods;
    Block server = getServerBlockWithPortAndName(ip, port, server_name);
    Block location_block = getLongestMatchingLocation(server, location);
    if (location_block.name.empty()) {
        return methods;  // TODO: throw
    }

    if (location_block.directives.find("limit_except") !=
        location_block.directives.end()) {
        std::stringstream ss(location_block.directives.at("limit_except"));
        std::string method;
        while (ss >> method) {
            methods.push_back(method);
        }
    }
    return methods;
}

bool Configuration::isDirectoryListingEnabled(
    const std::string& ip, const std::string& port,
    const std::string& server_name, const std::string& location) const {
    Block server = getServerBlockWithPortAndName(ip, port, server_name);
    Block location_block = getLongestMatchingLocation(server, location);

    if (location_block.name.empty()) {
        return false;
    }

    if (location_block.directives.find("autoindex") !=
        location_block.directives.end()) {
        if (location_block.directives.at("autoindex") == "on") {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

std::string Configuration::getErrorPageFromServer(
    const std::string& ip, const std::string& port,
    const std::string& server_name, const std::string& status_code) const {
    Block server = getServerBlockWithPortAndName(ip, port, server_name);
    if (server.name.empty()) {
        return "";
    }

    if (server.directives.find("error_page") != server.directives.end()) {
        std::string errors = server.directives.at("error_page");
        if (errors.find(status_code) != std::string::npos) {
            size_t last_space_pos = errors.rfind(' ');
            std::string path = errors.substr(last_space_pos + 1);
            return path;
        }
    }
    return "";
}

std::vector<std::string> Configuration::getIndexList(
    const std::string& ip, const std::string& port,
    const std::string& server_name, const std::string& location) const {
    std::vector<std::string> index_list;
    std::string list;
    Block server = getServerBlockWithPortAndName(ip, port, server_name);
    Block location_block = getLongestMatchingLocation(server, location);

    if (location_block.directives.find("index") !=
        location_block.directives.end()) {
        std::stringstream ss(location_block.directives.at("index"));
        std::string index;
        while (ss >> index) {
            index_list.push_back(index);
        }
    } else {
        index_list.push_back("index.html");
    }
    return index_list;
}

std::string Configuration::getCgiPath(const std::string& ip,
                                      const std::string& port,
                                      const std::string& server_name,
                                      const std::string& extension) const {
    std::string path;
    std::string ex;
    Block server = getServerBlockWithPortAndName(ip, port, server_name);
    if (server.directives.find("cgi") != server.directives.end()) {
        size_t separator_pos = server.directives.at("cgi").find(' ');
        ex = server.directives.at("cgi").substr(0, separator_pos);
        if (ex == extension) {
            path = server.directives.at("cgi").substr(separator_pos + 1);
        } else {
            path = "";
        }
    } else {
        path = "";
    }
    return path;
}