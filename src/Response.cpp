#include "Response.hpp"

Response::Response(int code, const std::string& content)
    : code(code), content(content) {}

int Response::getCode() const { return code; }
std::string Response::getContent() const { return content; }

std::string Response::toString() const {
    std::string response = "HTTP/1.1 ";
    response += std::to_string(code);
    response += " ";
    if (code == 200) {
        response += "OK";
    } else if (code == 404) {
        response += "Not Found";
    } else if (code == 405) {
        response += "Method Not Allowed";
    }
    response += "\r\nContent-Type: text/html\r\nContent-Length: ";
    response += std::to_string(content.length());
    response += "\r\n\r\n";
    response += content;
    return response;
}
