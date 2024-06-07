#include "Response.hpp"

std::string Response::messageGenerate(ResponseData data) {
    std::stringstream ss;

    ss << "HTTP/1.1 " << data.statusCode << " "
       << reasonPhrases[data.statusCode] << "\r\n";

    for (std::map<std::string, std::string>::iterator it = data.headers.begin();
         it != data.headers.end(); it++) {
        ss << it->first << ": " << it->second << "\r\n";
    }

    ss << "\r\n";

    ss << data.body;

    return ss.str();
}
