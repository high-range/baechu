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
    for (it = serverSokets.begin(); it != serverSokets.end(); it++) {
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

    // Set SO_REUSEADDR option
    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
        -1) {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        close(serverFd);
        return false;
    }

    if (bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) ==
        -1) {
        std::cerr << "Failed to bind socker" << std::endl;
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
        std::cerr << "Failed to add event" << std::endl;
        close(serverFd);
        return false;
    }

    serverSokets.push_back(serverFd);
    return true;
}

void Connector::start() {
    while (true) {
        struct kevent events[10];
        int nevents = kevent(kq, NULL, 0, events, 10, NULL);
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
        int fd = event.ident;

        if (std::find(serverSokets.begin(), serverSokets.end(), fd) !=
            serverSokets.end()) {
            // Accept new connection
            if (!acceptConnection(fd)) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
        } else {
            // Handle request
            handleRequest(fd);
        }
    }
}

bool Connector::acceptConnection(int serverFd) {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientFd =
        accept(serverFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd == -1) {
        std::cerr << "Failed to accept connection: " << strerror(errno)
                  << std::endl;
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

    std::cout << "\nConnection accepted" << std::endl;
    // 클라이언트 주소 정보 출력. TODO: 추후 삭제!!!
    std::cout << "Client IP: " << inet_ntoa(clientAddr.sin_addr) << std::endl;
    std::cout << "Client port: " << ntohs(clientAddr.sin_port) << std::endl;
    return true;
}

void Connector::handleRequest(int client_fd) {
    std::string request;
    char buffer[BUFFER_SIZE];

    while (true) {
        int bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_read < 0) {
            // Check if it's a non-blocking mode error (temporary condition)
            // or a genuine failure (permanent error)
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // std::cerr << "Data is not ready yet." << std::endl;
                break;
            } else {
                std::cerr << "Failed to read from socket: " << strerror(errno)
                          << std::endl;
                close(client_fd);
                return;
            }
        } else if (bytes_read == 0) {
            std::cerr << "Connection closed by client." << std::endl;
            break;
        }

        request.append(buffer, bytes_read);
        std::cout << bytes_read << " bytes read. Request message appended"
                  << std::endl;
    }

    // prepare request, serverAddr, and clientAddr
    sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    getsockname(client_fd, (struct sockaddr*)&serverAddr, &serverAddrLen);
    sockaddr_in clientAddr = clientAddresses[client_fd];

    std::string response =
        Manager::run(request, RequestData(serverAddr, clientAddr));
    send(client_fd, response.c_str(), response.size(), 0);
}

void Connector::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking" << std::endl;
        exit(EXIT_FAILURE);
    }
}
