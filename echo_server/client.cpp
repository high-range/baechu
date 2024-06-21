#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    // 소켓 디스크립터 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Socket creation failed" << endl;
        exit(EXIT_FAILURE);
    }

    // 서버 주소 설정
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // 서버 IP 주소 변환 및 설정
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported" << endl;
        exit(EXIT_FAILURE);
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        cerr << "Connection Failed" << endl;
        exit(EXIT_FAILURE);
    }

    // 입력을 받아서 서버에 메시지 전송 및 서버 응답 출력
    string input;
    while (true) {
        cout << "Enter message: ";
        getline(cin, input);
		if (cin.eof()) {
			cout << "Finish" << endl;
			break;
		}

        // 메시지를 서버로 전송
        send(sock, input.c_str(), input.length(), 0);

        // 서버로부터 응답 수신
        int read_count = read(sock, buffer, BUFFER_SIZE);
        buffer[read_count] = '\0';
        cout << "Echo from server: " << buffer << endl;

        // 버퍼 초기화
        memset(buffer, 0, BUFFER_SIZE);
    }

    // 소켓 종료
    close(sock);
    return 0;
}
