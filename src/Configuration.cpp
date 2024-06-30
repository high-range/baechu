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

Configuration::~Configuration() {
    if (configuration_ != NULL) delete configuration_;
    configuration_ = NULL;
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
        std::cerr << "Error: \"server\" has not \"listen\"" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!checkMethods()) {
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
                    if (!(value[0] >= '0' && value[0] <= '9')) {
                        std::cerr << "Error: \"client_max_body_size\" "
                                     "directive has invalid value"
                                  << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    for (size_t i = 0; i < value.length(); i++) {
                        if (!((value[i] >= '0' && value[i] <= '9') ||
                              value[i] == 'M') ||
                            ((i < value.length() - 1) && value[i] == 'M')) {
                            std::cerr << "Error: \"client_max_body_size\" "
                                         "directive has invalid value"
                                      << std::endl;
                            exit(EXIT_FAILURE);
                        }
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
                                    if (!isValidMethos(method)) {
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
    const std::string& port_number, const std::string& server_name) const {
    // port 번호와 server_name 일치하는 server 찾기
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_it =
                     block_it->sub_blocks.begin();
                 sub_it != block_it->sub_blocks.end(); ++sub_it) {
                if (sub_it->name == "server" &&
                    sub_it->directives.find("listen") !=
                        sub_it->directives.end() &&
                    sub_it->directives.at("listen") == port_number &&
                    sub_it->directives.find("server_name") !=
                        sub_it->directives.end() &&
                    sub_it->directives.at("server_name") == server_name) {
                    return *sub_it;
                }
            }
        }
    }
    // port 번호 일치하는 server 찾기
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_it =
                     block_it->sub_blocks.begin();
                 sub_it != block_it->sub_blocks.end(); ++sub_it) {
                if (sub_it->name == "server" &&
                    sub_it->directives.find("listen") !=
                        sub_it->directives.end() &&
                    sub_it->directives.at("listen") == port_number) {
                    return *sub_it;
                }
            }
        }
    }
    // server_name 일치하는 server 찾기
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_it =
                     block_it->sub_blocks.begin();
                 sub_it != block_it->sub_blocks.end(); ++sub_it) {
                if (sub_it->name == "server" &&
                    sub_it->directives.find("server_name") !=
                        sub_it->directives.end() &&
                    sub_it->directives.at("server_name") == server_name) {
                    return *sub_it;
                }
            }
        }
    }

    // 없으면 빈 block return
    return Block();
}

Block Configuration::getServerBlockWithPortHelper(
    const std::vector<Block>& blocks, const std::string& port_number) const {
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "server") {
            if (block_it->directives.find("listen") !=
                block_it->directives.end()) {
                if (block_it->directives.at("listen") == port_number) {
                    return *block_it;
                }
            }
        }

        // 재귀적 탐색
        Block find =
            getServerBlockWithPortHelper(block_it->sub_blocks, port_number);
        if (!find.name.empty()) {
            return find;
        }
    }
    return Block();  // 찾지 못한 경우 빈 블록 반환
}

Block Configuration::getServerBlockWithPort(
    const std::string& port_number) const {
    return getServerBlockWithPortHelper(blocks, port_number);
}

Block Configuration::getLocationBlockWithPort(
    const std::string& port_number, const std::string& location) const {
    // server_name과 port_number로 server Block 찾기
    Block server = getServerBlockWithPort(port_number);
    if (server.name.empty()) {
        return Block();
    }

    // 모든 location 블록을 검사
    int longest_count = 0;
    std::string longest_match;
    std::string root_directory;
    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name.find("location ") != std::string::npos) {
            // "location " 이후의 문자열
            std::string location_name = block_it->name.substr(9);
            if (location == location_name) {
                return *block_it;
            }
            int match_length =
                countMatchingPrefixLength(location_name, location);
            if (match_length > longest_count) {
                longest_count = match_length;
                longest_match = location_name;
            }
        }
    }

    // longest_match에 해당하는 location 블록 찾기
    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name == "location " + longest_match) {
            return *block_it;
        }
    }

    // 없을 경우 빈 block return
    return Block();
}

std::string Configuration::getRootDirectory(const std::string& path) const {
    std::string server_name;
    std::string port_number;
    std::string location;

    // server, port, location parsing
    size_t slash_pos = path.find('/');
    size_t colon_pos = path.find(':');
    if (slash_pos != std::string::npos) {
        location = path.substr(slash_pos);
        if (colon_pos == std::string::npos || colon_pos > slash_pos) {
            server_name = path.substr(0, slash_pos);
            port_number = "80";
        } else {
            server_name = path.substr(0, colon_pos);
            port_number = path.substr(colon_pos + 1, slash_pos - colon_pos - 1);
        }
    } else {
        location = "/";
        if (colon_pos == std::string::npos) {
            server_name = path;
            port_number = "80";
        } else {
            server_name = path.substr(0, colon_pos);
            port_number = path.substr(colon_pos + 1);
        }
    }

    // server_name과 port_number로 server Block 찾기
    Block server = getServerBlockWithPortAndName(port_number, server_name);
    if (server.name.empty()) {
        return "";
    }

    // 모든 location 블록을 검사
    int longest_count = 0;
    std::string longest_match;
    std::string root_directory;
    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name.find("location ") != std::string::npos) {
            // "location " 이후의 문자열
            std::string location_name = block_it->name.substr(9);
            if (location == location_name) {
                if (block_it->directives.find("root") !=
                    block_it->directives.end()) {
                    return block_it->directives.at("root");
                }
            }
            int match_length =
                countMatchingPrefixLength(location_name, location);
            if (match_length > longest_count) {
                longest_count = match_length;
                longest_match = location_name;
            }
        }
    }

    // longest_match에 해당하는 location 블록의 root 값을 설정
    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name == "location " + longest_match) {
            if (block_it->directives.find("root") !=
                block_it->directives.end()) {
                root_directory = block_it->directives.at("root");
                break;
            } else {
                // location 블록에 root가 없을 경우, server의 root 사용
                if (server.directives.find("root") != server.directives.end()) {
                    root_directory = server.directives.at("root");
                }
            }
        }
    }

    // 없을 경우 빈 문자열 return
    if (root_directory.empty()) {
        root_directory = "";
    }
    return root_directory;
}

std::string Configuration::getRootDirectory(
    const std::string& port_number, const std::string& location,
    const std::string& server_name) const {
    // server_name과 port_number로 server Block 찾기
    Block server = getServerBlockWithPortAndName(port_number, server_name);
    if (server.name.empty()) {
        return "";
    }

    // 모든 location 블록을 검사
    int longest_count = 0;
    std::string longest_match;
    std::string root_directory;
    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); block_it++) {
        if (block_it->name.find("location ") != std::string::npos) {
            std::string location_name = block_it->name.substr(9);
            if (location.empty()) {
                if (location_name == "/") {
                    if (block_it->directives.find("root") !=
                        block_it->directives.end()) {
                        return block_it->directives.at("root");
                    }
                }
                if (location_name.length() > longest_match.length()) {
                    longest_match = location_name;
                }
            } else {
                if (location == location_name) {
                    longest_match = location_name;
                    break;
                }
                int match_length =
                    countMatchingPrefixLength(location_name, location);
                if (match_length > longest_count) {
                    longest_count = match_length;
                    longest_match = location_name;
                }
            }
        }
    }

    // longest_match에 해당하는 location 블록의 root 값을 설정
    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name == "location " + longest_match) {
            if (block_it->directives.find("root") !=
                block_it->directives.end()) {
                root_directory = block_it->directives.at("root");
                break;
            } else {
                // location 블록에 root가 없을 경우, server의 root 사용
                if (server.directives.find("root") != server.directives.end()) {
                    root_directory = server.directives.at("root");
                }
            }
        }
    }

    // 없을 경우 빈 문자열 return
    if (root_directory.empty()) {
        root_directory = "";
    }
    return root_directory;
}

std::string Configuration::getClientMaxBodySize(
    const std::string& port_number, const std::string& location) const {
    // port_number로 server Block 찾기
    Block server = getServerBlockWithPort(port_number);
    if (server.name.empty()) {
        return "1M";
    }

    // 가장 긴 일치 경로를 찾기 위해 모든 location 블록을 검사
    int longest_count = 0;
    std::string longest_match;
    std::string client_max_body_size;
    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name.find("location ") == 0) {
            std::string location_name = block_it->name.substr(9);
            if (location == location_name) {
                if (block_it->directives.find("client_max_body_size") !=
                    block_it->directives.end()) {
                    client_max_body_size =
                        block_it->directives.at("client_max_body_size");
                    break;
                }
            }
            if (countMatchingPrefixLength(location_name, location) >
                longest_count) {
                longest_count =
                    countMatchingPrefixLength(location_name, location);
                longest_match = location_name;
                if (block_it->directives.find("client_max_body_size") !=
                    block_it->directives.end()) {
                    client_max_body_size =
                        block_it->directives.at("client_max_body_size");
                }
            }
        }
    }

    // location 블록에 설정이 없으면 server 블록 설정 사용
    if (client_max_body_size.empty()) {
        if (server.directives.find("client_max_body_size") !=
            server.directives.end()) {
            client_max_body_size = server.directives.at("client_max_body_size");
        }
    }

    // server 블록에도 설정이 없으면 http 블록 설정 사용
    if (client_max_body_size.empty()) {
        for (std::vector<Block>::const_iterator block_it = blocks.begin();
             block_it != blocks.end(); ++block_it) {
            if (block_it->name == "http") {
                if (block_it->directives.find("client_max_body_size") !=
                    block_it->directives.end()) {
                    client_max_body_size =
                        block_it->directives.at("client_max_body_size");
                }
                break;
            }
        }
    }

    // http 블록에도 없으면 default 1024
    if (client_max_body_size.empty()) {
        client_max_body_size = "1M";
    }

    return client_max_body_size;
}

std::string Configuration::getDefaultPort() const {
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "http") {
            for (std::vector<Block>::const_iterator sub_block_it =
                     block_it->sub_blocks.begin();
                 sub_block_it != block_it->sub_blocks.end(); ++sub_block_it) {
                if (sub_block_it->name == "server") {
                    return sub_block_it->directives.at("listen");
                }
            }
        }
    }
    return "80";  // TODO: throw
}

std::vector<std::string> Configuration::getCgiExtensions(
    const std::string& path) const {
    std::string server_name;
    std::string port_number;
    std::string location;
    std::vector<std::string> cgi_extensions;

    // server, port, location parsing
    size_t slash_pos = path.find('/');
    size_t colon_pos = path.find(':');
    if (slash_pos != std::string::npos) {
        location = path.substr(slash_pos);
        if (colon_pos == std::string::npos || colon_pos > slash_pos) {
            server_name = path.substr(0, slash_pos);
            port_number = "80";
        } else {
            server_name = path.substr(0, colon_pos);
            port_number = path.substr(colon_pos + 1, slash_pos - colon_pos - 1);
        }
    } else {
        location = "/";
        if (colon_pos == std::string::npos) {
            server_name = path;
            port_number = "80";
        } else {
            server_name = path.substr(0, colon_pos);
            port_number = path.substr(colon_pos + 1);
        }
    }

    // server_name과 port_number로 server Block 찾기
    Block server = getServerBlockWithPortAndName(port_number, server_name);
    if (server.name.empty()) {
        return cgi_extensions;
    }

    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name.find("location ") != std::string::npos) {
            std::string location_name = block_it->name.substr(9);
            if (location_name[0] == '~') {
                size_t dot_pos = location_name.rfind('.');
                std::string extension = location_name.substr(dot_pos);
                cgi_extensions.push_back(extension);
            }
        }
    }
    return cgi_extensions;
}

bool Configuration::isMethodAllowedFor(const std::string& port_number,
                                       const std::string& location,
                                       const std::string& method) const {
    Block location_block = getLocationBlockWithPort(port_number, location);
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

std::string Configuration::getAllowedMethods(
    const std::string& port_number, const std::string& location) const {
    Block location_block = getLocationBlockWithPort(port_number, location);
    if (location_block.name.empty()) {
        return "";  // TODO: throw
    }

    if (location_block.directives.find("limit_except") !=
        location_block.directives.end()) {
        return location_block.directives.at(
            "limit_except");  // TODO: vector로 만들기
    }
    return "";
}

bool Configuration::isDirectoryListingEnabled(
    const std::string& port_nubmer, const std::string& location) const {
    Block location_block = getLocationBlockWithPort(port_nubmer, location);
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
    const std::string& path) const {
    std::string server_name;
    std::string port_number;
    std::string location;

    // server, port, location parsing
    size_t slash_pos = path.find('/');
    size_t colon_pos = path.find(':');
    if (slash_pos != std::string::npos) {
        location = path.substr(slash_pos);
        if (colon_pos == std::string::npos || colon_pos > slash_pos) {
            server_name = path.substr(0, slash_pos);
            port_number = "80";
        } else {
            server_name = path.substr(0, colon_pos);
            port_number = path.substr(colon_pos + 1, slash_pos - colon_pos - 1);
        }
    } else {
        location = "/";
        if (colon_pos == std::string::npos) {
            server_name = path;
            port_number = "80";
        } else {
            server_name = path.substr(0, colon_pos);
            port_number = path.substr(colon_pos + 1);
        }
    }

    // server_name과 port_number로 server Block 찾기
    Block server = getServerBlockWithPortAndName(port_number, server_name);
    if (server.name.empty()) {
        return "";  // TODO: default error_page ?
    }

    if (server.directives.find("error_page") != server.directives.end()) {
        return server.directives.at("error_page");
    }
    return "";
}