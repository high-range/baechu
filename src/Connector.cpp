#include "Connector.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "ResponseData.hpp"

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

void Connector::connectServerPorts() {
    Configuration& config = Configuration::getInstance();
    std::vector<int> serverPorts = config.getPortNumbers();
    int serverFd;

    if (serverPorts.empty()) {
        throw std::runtime_error(
            "No server ports specified in configuration file");
    }
    for (size_t i = 0; i < serverPorts.size(); i++) {
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            continue;
        }

        setNonBlocking(serverFd);

        sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(serverPorts[i]);

        int opt = 1;
        if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
            -1) {
            std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
            close(serverFd);
            continue;
        }

        if (bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) ==
            -1) {
            std::cerr << "Failed to bind socket on port " << serverPorts[i]
                      << std::endl;
            close(serverFd);
            continue;
        }

        if (listen(serverFd, BACKLOG) == -1) {
            std::cerr << "Failed to listen on socket" << std::endl;
            close(serverFd);
            continue;
        }

        if (addReadEvent(serverFd, NULL) == false) {
            continue;
        }

        serverSockets.push_back(serverFd);
        std::cout << "Server started on port " << serverPorts[i] << std::endl;
    }
    if (serverSockets.empty()) {
        throw std::runtime_error("Failed to start server on any port");
    }
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
        if (event.udata == NULL) {
            acceptConnection(event);
        } else {
            handleRead(event);
        }
    } else if (event.filter == EVFILT_WRITE) {
        handleWrite(event);
    } else if (event.filter == EVFILT_TIMER) {
        handleTimer(event);
    }
}

void Connector::acceptConnection(struct kevent& event) {
    int serverFd = event.ident;
    int clientCount = event.data;

    while (clientCount > 0) {
        sockaddr_in serverAddr, clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        socklen_t serverAddrLen = sizeof(serverAddr);
        int clientFd =
            accept(serverFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientFd == -1) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        RequestData* requestData = new RequestData();

        setNonBlocking(clientFd);
        getsockname(clientFd, (struct sockaddr*)&serverAddr, &serverAddrLen);
        requestData->setClientData(clientAddr);
        requestData->setServerData(serverAddr);
        if (addReadEvent(clientFd, requestData) == false ||
            addTimerEvent(clientFd, requestData) == false) {
            delete requestData;
        }

        clientCount--;
    }
    // 클라이언트 주소 정보 출력. TODO: 추후 삭제
    // std::cout << "\nRequest came from " << inet_ntoa(clientAddr.sin_addr) <<
    // ":" << ntohs(clientAddr.sin_port) << std::endl;
}

void Connector::handleRead(struct kevent& event) {
    std::string request;
    RequestData& requestData = *static_cast<RequestData*>(event.udata);
    int clientFd = event.ident;
    int readSize = event.data;
    char buffer[BUFFER_SIZE];

    try {
        if (event.flags)
            while (readSize > 0) {
                std::memset(buffer, 0, sizeof(buffer));  // 버퍼 초기화
                int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
                if (bytesRead < 0) {
                    throw ResponseData(400);
                }
                request.append(buffer, bytesRead);
                readSize -= bytesRead;
            }

        requestData.appendData(request);

        if (Request::parseMessage(requestData)) {
            std::string* response = new std::string(Manager::run(requestData));

            delete &requestData;
            deleteReadEvent(clientFd);
            if (addWriteEvent(clientFd, response) == false) {
                delete response;
            }
        }
    } catch (ResponseData& responseData) {
        std::string temp = Manager::run(requestData, responseData);
        std::string* response = new std::string(temp);

        delete &requestData;
        deleteReadEvent(clientFd);
        if (addWriteEvent(clientFd, response) == false) {
            delete response;
        }
    }
    // Process the request and prepare the response
}

void Connector::handleWrite(struct kevent& event) {
    int clientFd = event.ident;
    std::string& response = *static_cast<std::string*>(event.udata);
    ssize_t bytesSent = send(clientFd, response.c_str(), response.size(), 0);

    if (bytesSent == -1) {
        std::cerr << "Failed to send response to client" << std::endl;
    } else if (static_cast<size_t>(bytesSent) == response.size()) {
        std::cout << "<Connection closed>" << std::endl;
    }
    delete &response;
    close(clientFd);
}

void Connector::handleTimer(struct kevent& event) {
    int clientFd = event.ident;
    RequestData& requestData = *static_cast<RequestData*>(event.udata);
    std::string temp = Manager::run(requestData, ResponseData(408));
    std::string* response = new std::string(temp);

    delete &requestData;
    deleteReadEvent(clientFd);
    addWriteEvent(clientFd, response);
}

void Connector::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking" << std::endl;
        exit(EXIT_FAILURE);
    }
}

bool Connector::addWriteEvent(int fd, void* udata) {
    struct kevent event;
    EV_SET(&event, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0,
           udata);
    if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Failed to add write event to kqueue" << std::endl;
        close(fd);
        return false;
    }
    return true;
}

bool Connector::addReadEvent(int fd, void* udata) {
    struct kevent event;
    EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
    if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Failed to add read event to kqueue" << std::endl;
        close(fd);
        return false;
    }
    return true;
}

bool Connector::addTimerEvent(int fd, void* udata) {
    struct kevent event;
    EV_SET(&event, fd, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT,
           NOTE_SECONDS, 30, udata);
    if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Failed to add timer event to kqueue" << std::endl;
        close(fd);
        return false;
    }
    return true;
}

void Connector::deleteReadEvent(int fd) {
    struct kevent event;
    EV_SET(&event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Failed to delete read event from kqueue" << std::endl;
    }
}
