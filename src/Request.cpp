#include "Request.hpp"

Request::Request(const std::string& message) {
    if (message == "POST") {
        method = "POST";
        url = "/test";
        host = "localhost:8080";
        body = "<html><body>test</body></html>";
        return;
    } else if (message == "GET1") {
        method = "GET";
        url = "/test";
        host = "localhost:8080";
        return;
    } else if (message == "GET2") {
        method = "GET";
        url = "/test/saved_file.html";
        host = "localhost:8080";
        return;
    } else if (message == "DELETE") {
        method = "DELETE";
        url = "/test/saved_file.html";
        host = "localhost:8080";
        return;
    }
}

std::string Request::getMethod() const { return method; }
std::string Request::getUrl() const { return url; }
std::string Request::getHost() const { return host; }
std::string Request::getBody() const { return body; }
