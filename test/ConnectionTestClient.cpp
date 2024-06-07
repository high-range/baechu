#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

void sendRequest(int sock, const std::string& message) {
    char buffer[1024] = {0};

    // 요청 보내기
    send(sock, message.c_str(), message.size(), 0);
    std::cout << "Request sent" << std::endl;

    // 응답 받기
    int valread = read(sock, buffer, 1024);
    std::cout << "Response received:\n" << buffer << std::endl;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    std::string message1 = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string message2 = "GET /another HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // 주소 변환
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // 첫 번째 요청 보내기
    sendRequest(sock, message1);

    // 두 번째 요청 보내기
    sendRequest(sock, message2);

    // 소켓 닫기
    close(sock);

    return 0;
}
