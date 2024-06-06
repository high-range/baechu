#include "Response.hpp"

std::string Response::messageGenerate(
    std::vector<std::pair<int, std::string> > responseMessageSource) {
    std::string response;
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    response += "<html><body><h1>Hello, World!</h1></body></html>";
    return response;
}
