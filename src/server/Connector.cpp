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

    if (listen(serverFd, BACKLOG) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(serverFd);
        return false;
    }

    struct kevent eventRead;
    EV_SET(&eventRead, serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kq, &eventRead, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Failed to add read event to kqueue" << std::endl;
        close(serverFd);
        return false;
    }

    // struct kevent eventWrite;
    // EV_SET(&eventWrite, serverFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0,
    // NULL); if (kevent(kq, &eventWrite, 1, NULL, 0, NULL) == -1) {
    //     std::cerr << "Failed to add write event to kqueue" << std::endl;
    //     close(serverFd);
    //     return false;
    // }

    serverSokets.push_back(serverFd);
    std::cout << "Server started on port " << port << std::endl;
    return true;
}

void Connector::start() {
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
            acceptConnection(event);
        } else {
            handleRead(event);
        }
    } else if (event.filter == EVFILT_WRITE) {
        handleWrite(event);
    }
}

bool Connector::acceptConnection(struct kevent& event) {
    int serverFd = event.ident;
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (event.data > 0) {
        int clientFd =
            accept(serverFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientFd == -1) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }
        event.data--;

        setNonBlocking(clientFd);
        struct kevent event;
        EV_SET(&event, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
            std::cerr << "Failed to add event" << std::endl;
            close(clientFd);
            continue;
        }

        // 클라이언트 주소 정보를 저장
        clientAddresses[clientFd] = clientAddr;

        // 클라이언트 주소 정보 출력. TODO: 추후 삭제
        std::cout << "\nRequest came from " << inet_ntoa(clientAddr.sin_addr)
                  << ":" << ntohs(clientAddr.sin_port) << std::endl;
    }
    return true;
}

void Connector::closeConnection(int clientFd) {
    close(clientFd);
    clientAddresses.erase(clientFd);
}

void Connector::handleRead(struct kevent& event) {
    int clientFd = event.ident;
    std::string request;
    char buffer[BUFFER_SIZE];

    while (event.data > 0) {
        std::memset(buffer, 0, sizeof(buffer));  // 버퍼 초기화
        int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

        if (bytesRead < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                std::cerr << "Failed to read from socket: " << strerror(errno)
                          << std::endl;
                closeConnection(clientFd);
                return;
            }
        } else if (bytesRead == 0) {
            // 클라이언트가 연결을 닫음
            closeConnection(clientFd);
            std::cout << "<Connection closed>" << std::endl;
            return;
        }

        request.append(buffer, bytesRead);
        std::cout << bytesRead << " bytes read (Request message appended)"
                  << std::endl;
        event.data--;
    }

    // Process the request and prepare the response
    sockaddr_in serverAddr, clientAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    getsockname(clientFd, (struct sockaddr*)&serverAddr, &serverAddrLen);
    clientAddr = clientAddresses[clientFd];
    std::string response =
        Manager::run(request, RequestData(serverAddr, clientAddr));

    clientResponses[clientFd] = response;

    struct kevent newEvent;
    EV_SET(&newEvent, clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0,
           0, NULL);
    if (kevent(kq, &newEvent, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Failed to add write event to kqueue" << std::endl;
        closeConnection(clientFd);
    }
}

void Connector::handleWrite(struct kevent& event) {
    int clientFd = event.ident;
    if (clientResponses.find(clientFd) == clientResponses.end()) {
        return;
    }

    std::string& response = clientResponses[clientFd];
    ssize_t bytesSent = send(clientFd, response.c_str(), response.size(), 0);

    if (bytesSent == -1) {
        std::cerr << "Failed to send response to client" << std::endl;
    }
    else if (static_cast<size_t>(bytesSent) == response.size()) {
        std::cout << "<Connection closed>" << std::endl;
    }
    clientResponses.erase(clientFd);
    closeConnection(clientFd);
}

void Connector::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking" << std::endl;
        exit(EXIT_FAILURE);
    }
}
