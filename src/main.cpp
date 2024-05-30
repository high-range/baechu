#include "RequestManager.hpp"

int main(void) {
    RequestData requestData;
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
    std::vector<std::pair<int, std::string> > statusData;

    statusData = RequestManager::parse(requestMessage, requestData);
    // 파싱 과정중 에러가 발생하면 vector 에 status code 와 message 가 저장되어
    // 반환됨 정상 동작시 status code 0 과 OK 가 저장되어 반환됨

    std::cout << requestData.getMethod() << std::endl;
    std::cout << requestData.getPath() << std::endl;
    std::cout << requestData.getVersion() << std::endl;
    std::map<std::string, std::vector<std::string> >::iterator it;
    std::map<std::string, std::vector<std::string> > myMap =
        requestData.getHeader();
    for (it = myMap.begin(); it != myMap.end(); ++it) {
        std::cout << "Key: " << it->first << "\n";
        for (size_t i = 0; i < it->second.size(); ++i) {
            std::cout << "Value" << i << " : " << it->second[i] << "\n";
        }
        std::cout << "\n";
    }
    // requestData.getHeader();
    // requestData.getBody();
}
