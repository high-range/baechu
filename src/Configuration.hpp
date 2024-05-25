#pragma once

#include <iostream>
class Configuration {
  public:
    static std::string getRootDirectory(std::string host);
    static std::string getCgiDirectory();
    static std::string setRootDirectory(std::string rootDirectory);
    static std::string setCgiDirectory(std::string cgiDirectory);

  private:
    static std::string rootDirectory;
    static std::string cgiDirectory;
};
