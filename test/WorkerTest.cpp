#include <iostream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "RequestData.hpp"
#include "Worker.hpp"

int main() {
    RequestData requestData;
    Configuration config;
    std::string requestMessage =
        "GET /? HTTP/1.1\r\nHost: localhost:8080\r\nConnection: "
        "keep-alive\r\nCache-Control: max-age=0\r\nsec-ch-ua: "
        "\"Chromium\";v=\"124\", \"Google Chrome\";v=\"124\", "
        "\"Not-A.Brand\";v=\"99\"\r\nsec-ch-ua-mobile: "
        "?0\r\nsec-ch-ua-platform: \"macOS\"\r\nUpgrade-Insecure-Requests: "
        "1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) "
        "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 "
        "Safari/537.36\r\nAccept: "
        "text/html,application/xhtml+xml,application/xml;q=0.9,image/"
        "avif,image/webp,image/apng,*/*;q=0.8,application/"
        "signed-exchange;v=b3;q=0.7\r\nSec-Fetch-Site: "
        "cross-site\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: "
        "?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br, "
        "zstd\r\nAccept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n";
    // HTTP request message

    Request::messageParse(requestMessage, requestData, config);
    Worker worker;

    std::vector<std::pair<int, std::string> > response =
        worker.handleRequest(requestData);

    for (auto& res : response) {
        std::cout << "**status code**\n\n" << res.first << std::endl;
        std::cout << "**response message**\n\n" << res.second << std::endl;
    }
    return 0;
}
