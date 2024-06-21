#include <iostream>

#include "../src/Configuration.hpp"

int main(int argc, char** argv) {
    std::string filename = "./conf/default.conf";

    // Get the single instance of Configuration and initialize it
    Configuration& config = Configuration::getInstance();
    config.initialize(filename);

    std::string path = "localhost:1000/a/b";
    std::string root = config.getRootDirectory(path);
    std::cout << root << std::endl;
    return 0;
}
