#pragma once

#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <vector>

// typedef std::vector<Block> vectorBlock;
// typedef std::map<std::string, std::string> mapDirective;

struct Block {
    std::string name;
    std::map<std::string, std::string> directives;
    std::vector<Block> sub_blocks;
};

class Configuration {
  public:
    // constructor
    Configuration();
    Configuration(const std::string& filename);

    // parsing
    void parseConfigFile(const std::string& filename);
    bool parseBlock(std::ifstream& file, Block& current_block);

    // format check
    bool isDouleKey();

    // 파싱 확인 함수 (지워야함)
    void printConfig() const;
    void printBlock(const Block& block, int indent) const;

    // get information
    Block getServerBlockWithName(const std::string& server_name) const;
    Block getServerBlockWithNameHelper(const std::vector<Block>& blocks,
                                       const std::string& server_name) const;
    Block getServerBlockWithPort(const std::string& port_number) const;
    Block getServerBlockWithPortHelper(const std::vector<Block>& blocks,
                                       const std::string& server_name) const;
    std::string getRootDirectory(const std::string& server_name,
                                 const std::string& port_number,
                                 const std::string& location) const;

  private:
    std::map<std::string, std::string> simple_directives;
    std::vector<Block> blocks;
};

std::string trim(const std::string& str);
bool isValidBlockName(const std::string& name);
bool isValidDirectiveKey(const std::string& key);
