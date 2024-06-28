#include "Response.hpp"

static std::string getReasonPhrase(int statusCode) {
    static std::map<int, std::string> reasonPhrases;
    if (reasonPhrases.empty()) {
        reasonPhrases[200] = "OK";
        reasonPhrases[201] = "Created";
        reasonPhrases[204] = "No Content";
        reasonPhrases[400] = "Bad Request";
        reasonPhrases[404] = "Not Found";
        reasonPhrases[405] = "Method Not Allowed";
        reasonPhrases[500] = "Internal Server Error";
        reasonPhrases[501] = "Not Implemented";
    }

    return reasonPhrases[statusCode];
}

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
       << getReasonPhrase(data.statusCode) << "\r\n";

    Headers headers = data.headers;
    headers["content-length"] = std::to_string(data.body.length());

    for (Headers::iterator it = headers.begin(); it != headers.end(); it++) {
        ss << camel(it->first) << ": " << it->second << "\r\n";
    }

    ss << "\r\n";

    ss << data.body;

    return ss.str();
}
