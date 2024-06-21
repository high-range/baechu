#include "Connector.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

Connector::Connector(int port) : port(port), serverFd(-1), kq(-1) {}

void Connector::start() {
    try {
        setupServer();
        std::cout << "Server started on port " << port << std::endl;
        handleConnections();
        std::cout << "Server stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        stop();
    } catch (const char* msg) {
        std::cerr << "Exception: " << msg << std::endl;
        stop();
    }
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
                int clientFd = accept(serverFd, nullptr, nullptr);
                std::cout << clientFd << "\n";
                if (clientFd == -1) {
                    std::cerr
                        << "Failed to accept connection: " << strerror(errno)
                        << std::endl;
                    continue;
                }
                setNonBlocking(clientFd);
                addEvent(clientFd, EVFILT_READ, EV_ADD);
            } else {
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
