#include "Connector.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

Connector::Connector(int port) : port(port), serverFd(-1), kq(-1) {}

void Connector::start() {
    setupServer();
    std::cout << "Server started on port " << port << std::endl;
    handleConnections();
    std::cout << "Server stopped" << std::endl;
}

void Connector::stop() {
    if (serverFd != -1) {
        close(serverFd);
    }
    if (kq != -1) {
        close(kq);
    }
}

void Connector::setupServer() {
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR option
    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
        -1) {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    setNonBlocking(serverFd);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) ==
        -1) {
        std::cerr << "Failed to bind socker" << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 10) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    kq = kqueue();
    if (kq == -1) {
        std::cerr << "Failed to create kqueue" << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    addEvent(serverFd, EVFILT_READ, EV_ADD);
}

void Connector::handleConnections() {
    struct kevent events[10];
    while (true) {
        int nev = kevent(kq, changes.data(), changes.size(), events, 10, NULL);
        changes.clear();
        if (nev == -1) {
            std::cerr << "kevent failed: " << strerror(errno) << std::endl;
            break;
        }

        for (int i = 0; i < nev; i++) {
            int fd = events[i].ident;
            if (fd == serverFd) {
                sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientFd = accept(serverFd, (struct sockaddr*)&clientAddr,
                                      &clientAddrLen);
                if (clientFd == -1) {
                    std::cerr
                        << "Failed to accept connection: " << strerror(errno)
                        << std::endl;
                    continue;
                }
                setNonBlocking(clientFd);
                addEvent(clientFd, EVFILT_READ, EV_ADD);

                // 클라이언트 주소 정보를 저장
                clientAddresses[clientFd] = clientAddr;

                // 클라이언트 IP 주소와 포트 번호 출력
                std::string ip_address = inet_ntoa(clientAddr.sin_addr);
                int port = ntohs(clientAddr.sin_port);
                std::cout << "\nClient IP Address: " << ip_address << std::endl;
                std::cout << "Client Port: " << port << std::endl;
            } else {
                // [클라이언트 주소 정보를 출력 (getpeername 사용 불가)]
                // sockaddr_in AddrForTest;
                // socklen_t AddrLenForTest = sizeof(AddrForTest);
                // getpeername(fd, (struct sockaddr*)&AddrForTest,
                //             &AddrLenForTest);
                // std::cout << "\nClient IP Address: "
                //           << inet_ntoa(AddrForTest.sin_addr) <<
                //           std::endl;
                // std::cout << "Client Port: "
                //           << ntohs(AddrForTest.sin_port) << std::endl;

                // [서버 주소 정보를 출력 (getsockname 사용 가능)]
                // sockaddr_in AddrForTest;
                // socklen_t AddrLenForTest = sizeof(AddrForTest);
                // getsockname(fd, (struct sockaddr*)&AddrForTest,
                //             &AddrLenForTest);
                // std::cout << "Local Port  : " << AddrForTest.sin_port
                //           << std::endl;
                // std::cout << "Local IP address: "
                //           << inet_ntoa(AddrForTest.sin_addr) <<
                //           std::endl;

                // [클라이언트 주소 정보를 출력 (clientAddresses 사용)]
                // sockaddr_in clientAddr = clientAddresses[fd];
                // std::string ip_address = inet_ntoa(clientAddr.sin_addr);
                // int port = ntohs(clientAddr.sin_port);
                // std::cout << "\nClient IP Address: " << ip_address <<
                // std::endl; std::cout << "Client Port: " << port << std::endl;
                handleRequest(fd);
                close(fd);
            }
        }
    }
}

void Connector::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Connector::addEvent(int fd, int filter, int flags) {
    struct kevent change;
    EV_SET(&change, fd, filter, flags, 0, 0, nullptr);
    changes.push_back(change);
}
