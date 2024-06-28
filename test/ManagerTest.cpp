#include <iostream>

#include "../src/Configuration.hpp"
#include "../src/Request.hpp"
#include "../src/RequestData.hpp"
#include "../src/ResponseData.hpp"

using namespace std;

int main(void) {
    RequestData requestData;
    Configuration config = Configuration::getInstance();
    // std::string requestMessage =
    //     "POST /upload?asdf HTTP/1.1\r\n"
    //     "Host: www.example.com\r\n"
    //     "User-Agent: CustomClient/1.0\r\n"
    //     "Content-Type: application/x-www-form-urlencoded\r\n"
    //     "Transfer-Encoding: chunked\r\n"
    //     "Connection: close\r\n"
    //     "\r\n"
    //     "7\r\n"
    //     "param1=\r\n"
    //     "8\r\n"
    //     "value1&pa\r\n"
    //     "6\r\n"
    //     "ram2=\r\n"
    //     "6\r\n"
    //     "value2\r\n"
    //     "0\r\n"
    //     "\r\n";
    // POST message test / transfer-encoding: chunked

    // std::string requestMessage =
    //     "POST /api/v1/resources HTTP/1.1\r\n"
    //     "Host: www.example.com\r\n"
    //     "User-Agent: CustomClient/1.0\r\n"
    //     "Content-Type: application/x-www-form-urlencoded\r\n"
    //     "content-length: 38\r\n"
    //     "Connection: close\r\n"
    //     "Accept: */*\r\n"
    //     "Accept-Encoding: gzip, deflate, br\r\n"
    //     "Accept-Language: en-US,en;q=0.9\r\n"
    //     "\r\n"
    //     "param1=value1&param2=value2&param3=value3";
    // POST message test / content-length

    std::string requestMessage =
        "POST / HTTP/1.1\r\n"
        "Content-Type: text/plain\r\n"
        "User-Agent: PostmanRuntime/7.39.0\r\n"
        "Accept: */*\r\n"
        "Cache-Control: no-cache\r\n"
        "Postman-Token: 3fb5a8f3-eae5-4a77-9e42-8730f853ac5e\r\n"
        "Host: localhost:8080\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Connection: keep-alive\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello";
    // std::string requestMessage =
    //     "GET /? HTTP/1.1\r\nHost: localhost:8080\r\nConnection: "
    //     "keep-alive\r\nCache-Control: max-age=0\r\nsec-ch-ua: "
    //     "\"Chromium\";v=\"124\", \"Google Chrome\";v=\"124\", "
    //     "\"Not-A.Brand\";v=\"99\"\r\nsec-ch-ua-mobile: "
    //     "?0\r\nsec-ch-ua-platform: \"macOS\"\r\nUpgrade-Insecure-Requests: "
    //     "1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) "
    //     "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 "
    //     "Safari/537.36\r\nAccept: "
    //     "text/html,application/xhtml+xml,application/xml;q=0.9,image/"
    //     "avif,image/webp,image/apng,*/*;q=0.8,application/"
    //     "signed-exchange;v=b3;q=0.7\r\nSec-Fetch-Site: "
    //     "cross-site\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: "
    //     "?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate,
    //     br, " "zstd\r\nAccept-Language:
    //     ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n";
    // GET message test

    try {
        Request::parseMessage(requestMessage, requestData);
    } catch (ResponseData& response) {
        cout << response.statusCode << endl;
    }

    std::cout << "Method: " << requestData.getMethod() << std::endl;
    std::cout << "RequestTarget: " << requestData.getRequestTarget()
              << std::endl;
    std::cout << "Path: " << requestData.getPath() << std::endl;
    std::cout << "Query: " << requestData.getQuery() << std::endl;
    std::cout << "Version: " << requestData.getVersion() << std::endl;
    std::cout << std::endl;
    std::map<std::string, std::string>::iterator it;
    std::map<std::string, std::string> myMap = requestData.getHeader();
    // requestData에서 getter를 통해 필요한 데이터를 가져옴
    // header는 map<string, string> 형태로 저장되어 있음

    for (it = myMap.begin(); it != myMap.end(); ++it) {
        std::cout << "KEY: " << it->first << "\n";
        std::cout << "VALUE: " << it->second << "\n\n";
    }
    // header의 key와 value를 출력하는 테스트 코드

    std::cout << requestData.getBody() << std::endl;
    return (0);
}
