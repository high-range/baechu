#include "Response.hpp"

std::string Response::messageGenerate(ResponseData data) {
    std::ostringstream ss;

    ss << "HTTP/1.1 " << data.statusCode << " "
       << reasonPhrases[data.statusCode] << "\r\n";

    Headers headers = data.headers;
    for (Headers::iterator it = headers.begin(); it != headers.end(); it++) {
        ss << it->first << ": " << it->second << "\r\n";
    }

    ss << "\r\n";

    ss << data.body;

    return ss.str();
}
