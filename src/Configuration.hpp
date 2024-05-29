#pragma once

#include <cstring>
#include <fstream>
#include <map>
#include <vector>

struct Block {
    std::string name;
    std::map<std::string, std::string> directives;
    std::vector<Block> sub_blocks;
};

class Configuration {
public:
    void parseConfigFile(const std::string filename);
    void printConfig() const; // 파싱 확인 함수 (지워야함)
    void parseBlock(std::ifstream& file, Block& current_block);

	// server_name 으로 port # 가져오기
	std::string getPortNumber(const std::string server_name) const;
	// server_name 으로 rootDirectory 가져오기
	// std::string getRootDirectory(const std::string server_name, const std::string location) const;

private:
    std::map<std::string, std::string> simple_directives;
    std::vector<Block> blocks;
};

std::string trim(const std::string& str);