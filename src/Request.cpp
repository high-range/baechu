#include "Request.hpp"

Request::Request(std::string request) {
    // Parse request
    // save method, url, host, body
}

std::string Request::getBody() { return "<html><body>test</body></html>"; }

std::string Request::getHost() { return "localhost:8080"; }

std::string Request::getMethod() { return "POST"; }

std::string Request::getUrl() { return "/test"; }
