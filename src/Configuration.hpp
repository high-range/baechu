#pragma once

#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <vector>

struct Block {
    std::string name;
    std::map<std::string, std::string> directives;
    std::vector<Block> sub_blocks;
};

class Configuration {
  public:
    // Static method to get the single instance of Configuration
    static Configuration& getInstance();

    // Method to initialize the Configuration with a filename
    void initialize(const std::string& filename);

    // get information
    std::string getRootDirectory(const std::string& path) const;
    std::string getRootDirectory(const std::string& server_name,
                                 const std::string& port_number,
                                 const std::string& location) const;
    std::string getClientMaxBodySize(const std::string& port_number) const;

  private:
    // Private constructor to prevent instantiation
    Configuration();

    static Configuration* configuration_;
    // parsing
    void parseConfigFile(const std::string& filename);
    bool parseBlock(std::ifstream& file, Block& current_block);
    Block getServerBlockWithName(const std::string& server_name) const;
    Block getServerBlockWithNameHelper(const std::vector<Block>& blocks,
                                       const std::string& server_name) const;
    Block getServerBlockWithPort(const std::string& port_number) const;
    Block getServerBlockWithPortHelper(const std::vector<Block>& blocks,
                                       const std::string& port_number) const;
    std::map<std::string, std::string> simple_directives;
    std::vector<Block> blocks;
};

std::string trim(const std::string& str);
bool isValidBlockName(const std::string& name);
bool isValidDirectiveKey(const std::string& key);
int countMatchingPrefixLength(const std::string& location,
                              const std::string& request_location);
