#include "Connector.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

Connector::Connector() {
    kq = kqueue();
    if (kq == -1) {
        std::cerr << "Failed to create kqueue" << std::endl;
        exit(EXIT_FAILURE);
    }
}

Connector::~Connector() {
    std::vector<int>::iterator it;
    for (it = serverSockets.begin(); it != serverSockets.end(); it++) {
        close(*it);
    }
    close(kq);
}

bool Connector::addServer(int port) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    setNonBlocking(serverFd);

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
        -1) {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        close(serverFd);
        return false;
    }

    if (bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) ==
        -1) {
        std::cerr << "Failed to bind socket on port " << port << std::endl;
        close(serverFd);
        return false;
    }

    if (listen(serverFd, 10) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(serverFd);
        return false;
    }

    struct kevent event;
    EV_SET(&event, serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Failed to add event to kqueue" << std::endl;
        close(serverFd);
        return false;
    }

    serverSockets.push_back(serverFd);
    std::cout << "Server started on port " << port << std::endl;
    return true;
}

void Connector::start() {
    if (serverSockets.empty()) {
        std::cerr << "No servers have been added." << std::endl;
        return;
    }

    while (true) {
        struct kevent events[MAX_EVENTS];
        int nevents = kevent(kq, NULL, 0, events, MAX_EVENTS, NULL);
        if (nevents == -1) {
            std::cerr << "kevent failed while waiting for events" << std::endl;
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nevents; ++i) {
            handleEvent(events[i]);
        }
    }
}

void Connector::handleEvent(struct kevent& event) {
    if (event.filter == EVFILT_READ) {
        if (clientAddresses.find(event.ident) == clientAddresses.end()) {
            // 새로운 클라이언트 연결 수락
            if (acceptConnection(event.ident)) {
                std::cout << "<Connection accepted>" << std::endl;
            }
        } else {
            // 기존 클라이언트로부터의 요청 처리
            handleRequest(event.ident);
        }
    }
}

bool Connector::acceptConnection(int serverFd) {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientFd =
        accept(serverFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd == -1) {
        std::cerr << "Failed to accept connection" << std::endl;
        return false;
    }

    setNonBlocking(clientFd);

    struct kevent event;
    EV_SET(&event, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Failed to add event" << std::endl;
        close(clientFd);
        return false;
    }

    // 클라이언트 주소 정보를 저장
    clientAddresses[clientFd] = clientAddr;

    std::cout << "\n<Connection accepted>" << std::endl;
    // 클라이언트 주소 정보 출력. TODO: 추후 삭제
    std::cout << "Request came from " << inet_ntoa(clientAddr.sin_addr) << ":"
              << ntohs(clientAddr.sin_port) << std::endl;
    return true;
}

void Connector::closeConnection(int clientFd) {
    close(clientFd);
    clientAddresses.erase(clientFd);
}

void Connector::handleRequest(int clientFd) {
    std::string request;
    char buffer[BUFFER_SIZE];

    while (true) {
        int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

        if (bytesRead < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                std::cerr << "Failed to read from socket" << std::endl;
                closeConnection(clientFd);
                return;
            }
        } else if (bytesRead == 0) {
            closeConnection(clientFd);
            std::cout << "<Connection closed>" << std::endl;
            return;
        }

        request.append(buffer, bytesRead);
        std::cout << bytesRead << " bytes read (Request message appended)"
                  << std::endl;
    }

    sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    getsockname(clientFd, (struct sockaddr*)&serverAddr, &serverAddrLen);
    sockaddr_in clientAddr = clientAddresses[clientFd];

    std::string response =
        Manager::run(request, RequestData(serverAddr, clientAddr));

    size_t totalBytesSent = 0;
    while (totalBytesSent < response.size()) {
        ssize_t bytesSent = send(clientFd, response.c_str() + totalBytesSent,
                                 response.size() - totalBytesSent, 0);
        if (bytesSent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                std::cerr << "Failed to send response to client" << std::endl;
                break;
            }
        }
        totalBytesSent += bytesSent;
    }

    // if (totalBytesSent == response.size()) {
    //     std::cout << "Response sent to " << inet_ntoa(clientAddr.sin_addr)
    //               << ":" << ntohs(clientAddr.sin_port) << std::endl;
    // }

    closeConnection(clientFd);
    std::cout << "<Connection closed>" << std::endl;
}

void Connector::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking" << std::endl;
        exit(EXIT_FAILURE);
    }
}
