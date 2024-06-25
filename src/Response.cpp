#include "Response.hpp"

static std::string camel(std::string s) {
    bool capitalize = true;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '-') {
            capitalize = true;
        } else if (capitalize) {
            s[i] = toupper(s[i]);
            capitalize = false;
        }
    }
    return s;
}

std::string Response::messageGenerate(ResponseData data) {
    std::ostringstream ss;

    ss << "HTTP/1.1 " << data.statusCode << " "
       << reasonPhrases[data.statusCode] << "\r\n";

    Headers headers = data.headers;
    headers["content-length"] = std::to_string(data.body.length());

    for (Headers::iterator it = headers.begin(); it != headers.end(); it++) {
        ss << camel(it->first) << ": " << it->second << "\r\n";
    }

    ss << "\r\n";

    ss << data.body;

    return ss.str();
}
