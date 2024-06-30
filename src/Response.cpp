#include "Response.hpp"

static std::string getReasonPhrase(int statusCode) {
    static std::map<int, std::string> reasonPhrases;
    if (reasonPhrases.empty()) {
        // 1xx Informational
        reasonPhrases[100] = "Continue";
        reasonPhrases[101] = "Switching Protocols";
        reasonPhrases[102] = "Processing";
        reasonPhrases[103] = "Early Hints";

        // 2xx Success
        reasonPhrases[200] = "OK";
        reasonPhrases[201] = "Created";
        reasonPhrases[202] = "Accepted";
        reasonPhrases[203] = "Non-Authoritative Information";
        reasonPhrases[204] = "No Content";
        reasonPhrases[205] = "Reset Content";
        reasonPhrases[206] = "Partial Content";
        reasonPhrases[207] = "Multi-Status";
        reasonPhrases[208] = "Already Reported";
        reasonPhrases[226] = "IM Used";

        // 3xx Redirection
        reasonPhrases[300] = "Multiple Choices";
        reasonPhrases[301] = "Moved Permanently";
        reasonPhrases[302] = "Found";
        reasonPhrases[303] = "See Other";
        reasonPhrases[304] = "Not Modified";
        reasonPhrases[305] = "Use Proxy";
        reasonPhrases[306] = "unused";
        reasonPhrases[307] = "Temporary Redirect";
        reasonPhrases[308] = "Permanent Redirect";

        // 4xx Client Error
        reasonPhrases[400] = "Bad Request";
        reasonPhrases[401] = "Unauthorized";
        reasonPhrases[402] = "Payment Required";
        reasonPhrases[403] = "Forbidden";
        reasonPhrases[404] = "Not Found";
        reasonPhrases[405] = "Method Not Allowed";
        reasonPhrases[406] = "Not Acceptable";
        reasonPhrases[407] = "Proxy Authentication Required";
        reasonPhrases[408] = "Request Timeout";
        reasonPhrases[409] = "Conflict";
        reasonPhrases[410] = "Gone";
        reasonPhrases[411] = "Length Required";
        reasonPhrases[412] = "Precondition Failed";
        reasonPhrases[413] = "Payload Too Large";
        reasonPhrases[414] = "URI Too Long";
        reasonPhrases[415] = "Unsupported Media Type";
        reasonPhrases[416] = "Range Not Satisfiable";
        reasonPhrases[417] = "Expectation Failed";
        reasonPhrases[418] = "I'm a teapot";
        reasonPhrases[421] = "Misdirected Request";
        reasonPhrases[422] = "Unprocessable Content";
        reasonPhrases[423] = "Locked";
        reasonPhrases[424] = "Failed Dependency";
        reasonPhrases[425] = "Too Early";
        reasonPhrases[426] = "Upgrade Required";
        reasonPhrases[428] = "Precondition Required";
        reasonPhrases[429] = "Too Many Requests";
        reasonPhrases[431] = "Request Header Fields Too Large";
        reasonPhrases[451] = "Unavailable For Legal Reasons";

        // 5xx Server Error
        reasonPhrases[500] = "Internal Server Error";
        reasonPhrases[501] = "Not Implemented";
        reasonPhrases[502] = "Bad Gateway";
        reasonPhrases[503] = "Service Unavailable";
        reasonPhrases[504] = "Gateway Timeout";
        reasonPhrases[505] = "HTTP Version Not Supported";
        reasonPhrases[506] = "Variant Also Negotiates";
        reasonPhrases[507] = "Insufficient Storage";
        reasonPhrases[508] = "Loop Detected";
        reasonPhrases[510] = "Not Extended";
        reasonPhrases[511] = "Network Authentication Required";
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

    ss << "HTTP/1.1 " << data.statusCode;
    if (data.reasonPharse.empty()) {
        ss << " " << getReasonPhrase(data.statusCode);
    } else {
        ss << data.reasonPharse;  // containing leading space
    }
    ss << "\r\n";

    Headers headers = data.headers;
    headers["content-length"] = std::to_string(data.body.length());

    for (Headers::iterator it = headers.begin(); it != headers.end(); it++) {
        ss << camel(it->first) << ": " << it->second << "\r\n";
    }

    ss << "\r\n";

    ss << data.body;

    return ss.str();
}
