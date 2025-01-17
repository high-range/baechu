#pragma once

#include "Utils.hpp"

#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <exception>

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
    int getDefaultPort() const;
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
    std::vector<int> getPortNumbers() const;
    std::vector<std::string> getIndexList(const std::string& ip,
                                          const std::string& port,
                                          const std::string& server_name,
                                          const std::string& location) const;
    std::vector<std::string> getCgiExtensions(
        const std::string& ip, const std::string& port,
        const std::string& server_name) const;
    std::string getCgiPath(const std::string& ip, const std::string& port,
                           const std::string& server_name,
                           const std::string& extension) const;
	std::string getInterpreterPath(const std::string& ip,
								   const std::string& port,
								   const std::string& server_name,
								   const std::string& extension) const;
    bool isLocationHaveRedirect(const std::string& ip, const std::string& port,
                                const std::string& server_name,
                                const std::string& location) const;
    std::vector<std::string> getRedirectionInfo(
        const std::string& ip, const std::string& port,
        const std::string& server_name, const std::string& location) const;

  private:
    // Private constructor to prevent instantiation
    Configuration();

    static Configuration* configuration_;
    // parsing
    void parseConfigFile(const std::string& filename);
    bool parseBlock(std::ifstream& file, Block& current_block);

    // get block
    Block getServerBlockWithPortAndName(const std::string& ip,
                                        const std::string& port,
                                        const std::string& server_name) const;
    Block getLongestMatchingLocation(const Block& server,
                                     const std::string& request_location) const;

    // error check
    bool isServerHavePort() const;
    bool checkErrorPage() const;
    bool isValidListen(const std::string& listen_value) const;
    bool isValidCgiPath() const;
    bool isDuplicatedHttp() const;
    bool isValidRedirect() const;

    std::map<std::string, std::string> simple_directives;
    std::vector<Block> blocks;
};

std::string trim(const std::string& str);
bool isValidFile(const std::string& file);
bool isValidBlockName(const std::string& name);
bool isValidDirectiveKey(const std::string& key);
bool isValidKeyInBlock(const std::string& block_name, const std::string& key);
bool isValidMethods(const std::string& method);
int countMatchingPrefixLength(const std::string& location,
                              const std::string& request_location);
