#pragma once

#include <iostream>

class Configuration;

class Manager {
  public:
    virtual ~Manager() = 0;
    static std::string run(std::string requestMessage,
                           const Configuration& configuration);
};
