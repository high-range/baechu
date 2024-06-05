#include "Configuration.hpp"

#include <iostream>

// --------------------------------------- Parsing 함수
// ---------------------------------------------------
void Configuration::parseConfigFile(const std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }
    std::string line;
    Block main_block;

    while (std::getline(file, line)) {
        // 앞,뒤 공백 제거
        line = trim(line);

        // 빈 줄이거나, 주석이면 pass
        if (line.empty() || line[0] == '#') continue;

        // 블록 시작
        if (line.find("{") != std::string::npos) {
            Block block;

            size_t pos = line.find("{");
            block.name = trim(line.substr(0, pos));
            parseBlock(file, block);
            blocks.push_back(block);
        } else {
            size_t space_pos = line.find(' '), tap_pos = line.find('\t');
            size_t start_pos = std::min(space_pos, tap_pos);
            size_t end_pos = line.find(';');
            if (start_pos != std::string::npos) {
                std::string key = trim(line.substr(0, start_pos));
                std::string value =
                    trim(line.substr(start_pos + 1, end_pos - start_pos - 1));
                simple_directives[key] = value;
            }
        }
    }
}

void Configuration::parseBlock(std::ifstream& file, Block& current_block) {
    std::string line;

    while (std::getline(file, line)) {
        // 앞,뒤 공백 제거
        line = trim(line);

        // 빈 줄이거나, 주석이면 pass
        if (line.empty() || line[0] == '#') continue;

        if (line.find("{") != std::string::npos) {
            Block block;
            size_t pos = line.find('{');
            if (pos != std::string::npos) {
                block.name = trim(line.substr(0, pos));
                parseBlock(file, block);
                current_block.sub_blocks.push_back(block);
            }
        } else if (line.find('}') != std::string::npos) {
            return;
        } else {
            size_t space_pos = line.find(' '), tap_pos = line.find('\t');
            size_t start_pos = std::min(space_pos, tap_pos);
            size_t end_pos = line.find(';');
            if (start_pos != std::string::npos) {
                std::string key = trim(line.substr(0, start_pos));
                std::string value =
                    trim(line.substr(start_pos + 1, end_pos - start_pos - 1));
                current_block.directives[key] = value;
            }
        }
    }
}

void Configuration::printConfig() const {
    for (std::map<std::string, std::string>::const_iterator directive_it =
             simple_directives.begin();
         directive_it != simple_directives.end(); ++directive_it) {
        std::cout << directive_it->first << ": " << directive_it->second
                  << std::endl;
    }
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        std::cout << "Block name: " << block_it->name << std::endl;
        for (std::map<std::string, std::string>::const_iterator directive_it =
                 block_it->directives.begin();
             directive_it != block_it->directives.end(); ++directive_it) {
            std::cout << directive_it->first << ": " << directive_it->second
                      << std::endl;
        }
        for (std::vector<Block>::const_iterator sub_block_it =
                 block_it->sub_blocks.begin();
             sub_block_it != block_it->sub_blocks.end(); ++sub_block_it) {
            std::cout << "Sub block name: " << sub_block_it->name << std::endl;
            for (std::map<std::string, std::string>::const_iterator
                     sub_directive_it = sub_block_it->directives.begin();
                 sub_directive_it != sub_block_it->directives.end();
                 ++sub_directive_it) {
                std::cout << "  " << sub_directive_it->first << ": "
                          << sub_directive_it->second << std::endl;
            }
            for (std::vector<Block>::const_iterator subsub_block_it =
                     sub_block_it->sub_blocks.begin();
                 subsub_block_it != sub_block_it->sub_blocks.end();
                 ++subsub_block_it) {
                std::cout << " SubSub block name: " << subsub_block_it->name
                          << std::endl;
                for (std::map<std::string, std::string>::const_iterator
                         subsub_directive_it =
                             subsub_block_it->directives.begin();
                     subsub_directive_it != subsub_block_it->directives.end();
                     ++subsub_directive_it) {
                    std::cout << "   " << subsub_directive_it->first << ": "
                              << subsub_directive_it->second << std::endl;
                }
            }
        }
        std::cout << std::endl;
    }
}

// --------------------------------------- 정보 가져오는 함수
// ---------------------------------------------------
std::string Configuration::getPortNumber(const std::string server_name) const {
    for (std::vector<Block>::const_iterator block_it = blocks.begin();
         block_it != blocks.end(); ++block_it) {
        if (block_it->name == "server") {
            if (block_it->directives.find("server_name") !=
                block_it->directives.end()) {
                if (block_it->directives.at("server_name") == server_name) {
                    return block_it->directives.at("listen");
                }
            }
        }
    }
    return blocks[0].directives.at(
        "listen");  // return default port# - 기본값은 첫 번째 server의
                    // port#라고 명시되어 있음
}
