#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/event.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 10

using namespace std;

void set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int server_fd, new_socket, kq, nev;
	struct kevent change_list[MAX_EVENTS], event_list[MAX_EVENTS];
    struct sockaddr_in address; // 주소체계 구조체
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // 소켓 디스크립터 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 주소 구조체 설정
    address.sin_family = AF_INET;			// IPv4 통신, 기본 프로토콜 지정
    address.sin_addr.s_addr = INADDR_ANY;   // 서버의 IP주소를 자동으로 찾아서 대입
    address.sin_port = htons(PORT);         // 리틀 엔디안 -> 빅 엔디안

    // 소켓에 주소를 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 연결 대기열 설정
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

	// kqueue 생성
	if ((kq = kqueue()) == -1) {
		perror("kqueue");
		exit(EXIT_FAILURE);
	}

	// 서버 소켓을 non-blocking 모드로 설정
	set_nonblocking(server_fd);

	// 서버 소켓을 kqueue에 등록하여 일기 이벤트를 모니터링
	EV_SET(&change_list[0], server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    cout << "Echo server running on port " << PORT << endl;

    // 클라이언트 연결 수락
    while (true) {
		// 이벤트를 기다림
		nev = kevent(kq, change_list, 1, event_list, MAX_EVENTS, NULL);

		if (nev < 0) {
			perror("kevent error");
			exit(EXIT_FAILURE);
		}
		else if (nev == 0) {
			continue; // 타임아웃
		}

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        cout << "Client connected" << endl;

        // 클라이언트와 데이터 송수신
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int read_count = read(new_socket, buffer, BUFFER_SIZE);
            if (read_count <= 0) {
                cout << "Client disconnected" << endl;
                close(new_socket);
                break;
            }
            cout << "Received: " << buffer << endl;

            // 받은 데이터를 클라이언트로 다시 전송
            send(new_socket, buffer, read_count, 0);
            cout << "Echoed back: " << buffer << endl;
        }
    }
	close(server_fd);
    return 0;
}

class requestMsg {
public:

private:
};

typedef struct requestMsg {
	std::string version;
	std::string method;
};