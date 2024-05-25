#pragma once

#include <iostream>

class Request {
    public:
        std::string getMethod();
        std::string getUrl();
        std::string getHost();
        std::string getBody();

    private:
        std::string method;
        std::string url;
        std::string host;
        std::string body;
};
