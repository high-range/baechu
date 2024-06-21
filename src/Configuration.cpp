#include "Configuration.hpp"

#include <iostream>

// -------------------------- Constructor -------------------------------
Configuration::Configuration() {}

// Static method to get the single instance of Configuration
Configuration& Configuration::getInstance() {
    static Configuration instance;
    return instance;
}

// Method to initialize the Configuration with a filename
void Configuration::initialize(const std::string& filename) {
    parseConfigFile(filename);
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
            if (!isValidBlockName(block_name)) {
                std::cerr << "Invalid block name: " << block_name << std::endl;
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
            size_t start_pos = std::min(space_pos, tap_pos);
            size_t end_pos = line.find(';');

            if (end_pos == std::string::npos) {
                if (end_pos == std::string::npos) {
                    std::cerr << "Error: Directive not finished with ';'"
                              << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            if (start_pos != std::string::npos) {
                std::string key = trim(line.substr(0, start_pos));
                if (!isValidDirectiveKey(key)) {
                    std::cerr << "Invalid directive key: " << key << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::string value =
                    trim(line.substr(start_pos + 1, end_pos - start_pos - 1));
                simple_directives[key] = value;
            }
        }
    }
}

bool Configuration::parseBlock(std::ifstream& file, Block& current_block) {
    std::string line;
    int openBraces = 1;

    while (std::getline(file, line)) {
        // 앞,뒤 공백 제거
        line = trim(line);

        // 빈 줄이거나, 주석이면 pass
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line.find("{") != std::string::npos) {
            std::string block_name = trim(line.substr(0, line.find('{')));
            if (!isValidBlockName(block_name)) {
                std::cerr << "Invalid block name: " << block_name << std::endl;
                exit(EXIT_FAILURE);
            }
            Block block;
            block.name = block_name;
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
            size_t start_pos = std::min(space_pos, tap_pos);
            size_t end_pos = line.find(';');

            if (end_pos == std::string::npos) {
                std::cerr << "Error: Directive not finished with ';'"
                          << std::endl;
                exit(EXIT_FAILURE);
            }
            if (start_pos != std::string::npos) {
                std::string key = trim(line.substr(0, start_pos));
                if (!isValidDirectiveKey(key)) {
                    std::cerr << "Invalid directive key: " << key << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::string value =
                    trim(line.substr(start_pos + 1, end_pos - start_pos - 1));
                current_block.directives[key] = value;
            }
        }
    }
    if (openBraces != 0) {
        std::cerr << "Error: Block not closed." << std::endl;
        exit(EXIT_FAILURE);
    }
    return true;
}

// -------------------------- 정보 가져오는 함수 -------------------------------
Block Configuration::getServerBlockWithNameHelper(
    const std::vector<Block>& blocks, const std::string& server_name) const {
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "server") {
            if (block_it->directives.find("server_name") !=
                block_it->directives.end()) {
                if (block_it->directives.at("server_name") == server_name) {
                    return *block_it;
                }
            }
        }

        // 재귀적 탐색
        Block find =
            getServerBlockWithNameHelper(block_it->sub_blocks, server_name);
        if (!find.name.empty()) {
            return find;
        }
    }
    return Block();  // 찾지 못한 경우 빈 블록 반환
}

Block Configuration::getServerBlockWithName(
    const std::string& server_name) const {
    return getServerBlockWithNameHelper(blocks, server_name);
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

std::string Configuration::getRootDirectory(const std::string& server_name,
                                            const std::string& port_number,
                                            const std::string& location) const {
    // server_name과 port_number로 server Block 찾기
    // priority은 port_number > server_name
    Block server = Configuration::getServerBlockWithPort(port_number);
    if (server.name.empty()) {
        server = Configuration::getServerBlockWithName(server_name);
        if (server.name.empty()) {
            return "";
        }
    }

    // location Block 찾기
    std::string location_name = "location " + location;
    for (std::vector<Block>::const_iterator block_it =
             server.sub_blocks.begin();
         block_it != server.sub_blocks.end(); ++block_it) {
        if (block_it->name == location_name) {
            return block_it->directives.at("root");
        }
    }
    return "";
}
