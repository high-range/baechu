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
    std::string getRootDirectory(const std::string& ip, const std::string& port,
                                 const std::string& server_name,
                                 const std::string& location) const;
    std::string getClientMaxBodySize(const std::string& ip,
                                     const std::string& port,
                                     const std::string& server_name,
                                     const std::string& location) const;
    std::string getDefaultPort() const;
    bool isMethodAllowedFor(const std::string& ip, const std::string& port,
                            const std::string& server_name,
                            const std::string& location,
                            const std::string& method) const;
    std::vector<std::string> getAllowedMethods(
        const std::string& ip, const std::string& port,
        const std::string& server_name, const std::string& location) const;
    bool isDirectoryListingEnabled(const std::string& ip,
                                   const std::string& port,
                                   const std::string& server_name,
                                   const std::string& location) const;
    std::string getErrorPageFromServer(const std::string& ip,
                                       const std::string& port,
                                       const std::string& server_name,
                                       const std::string& status_code) const;
    std::vector<std::string> getPortNumbers() const;
    std::vector<std::string> getIndexList(const std::string& ip,
                                          const std::string& port,
                                          const std::string& server_name,
                                          const std::string& location) const;
    std::string getCgiPath(const std::string& ip, const std::string& port,
                           const std::string& server_name,
                           const std::string& extension) const;

  private:
    // Private constructor to prevent instantiation
    Configuration();

    static Configuration* configuration_;
    // parsing
    void parseConfigFile(const std::string& filename);
    bool parseBlock(std::ifstream& file, Block& current_block);
    Block getServerBlockWithPortAndName(const std::string& ip,
                                        const std::string& port,
                                        const std::string& server_name) const;

    // error check
    bool hasServerBlocks(const std::vector<Block>& blocks) const;
    bool isValidServerBlockPlacement(const std::vector<Block>& blocks,
                                     const std::string& upper_block) const;
    bool isServerHavePort() const;
    bool checkMethods() const;
    bool checkErrorPage() const;
    bool isValidListen(const std::string& listen_value) const;
    bool isValidCgiPath() const;
    bool isDuplicatedHttp() const;

    Block getLongestMatchingLocation(const Block& server,
                                     const std::string& request_location) const;

    std::map<std::string, std::string> simple_directives;
    std::vector<Block> blocks;
};

std::string trim(const std::string& str);
bool isValidBlockName(const std::string& name);
bool isValidDirectiveKey(const std::string& key);
bool isValidKeyInBlock(const std::string& block_name, const std::string& key);
bool isValidMethods(const std::string& method);
int countMatchingPrefixLength(const std::string& location,
                              const std::string& request_location);
