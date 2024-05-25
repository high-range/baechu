#include "Worker.hpp"

#include <sys/stat.h>

#include <cstdio>
#include <fstream>
#include <iostream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "Response.hpp"

/*
Worker::Worker() {
    // Constructor
}

Worker::~Worker() {
    // Destructor
}

Worker::Worker(const Worker& copy) {
    // Copy constructor
}

Worker& Worker::operator=(const Worker& copy) {
    // Copy assignment operator
}
*/

std::string Worker::getPath(Request request) {
    // Get path
    std::string rootDirectory =
        Configuration::getRootDirectory(request.getHost());
    return (rootDirectory + request.getUrl());
}

bool Worker::isStaticRequest(Request request) {
    // Check if request is static
    return true;
}

Response Worker::handelStaticRequest(Request request) {
    // Handle static request
    std::string method = request.getMethod();
    if (method == "GET") {
        doGet(request);
    } else if (method == "POST") {
        doPost(request);
    } else if (method == "DELETE") {
        doDelete(request);
    } else {
        // Return 405 Method Not Allowed for unsupported methods
        // Response(405, "Method Not Allowed");
    }
    return Response();
}

void Worker::doGet(Request request) {
    // Handle GET request
    std::string path = getPath(request);
}

void Worker::doPost(Request request) {
    std::string path = getPath(request);
    std::ofstream file(path);
    if (file.is_open()) {
        file << request.getBody();  // Write the request body to the file
        file.close();
        return;  // Return 201 if successful
    } else {
        return;  // Return 500 Internal Server Error
    }
}

void Worker::doDelete(Request request) {
    std::string path = getPath(request);
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        if (std::remove(path.c_str()) == 0) {
            return;  // Return 200 OK if successful
        }
    } else {
        return;  // Return 404 Not Found if the file does not exist
    }
}

Response Worker::handleRequest(Request request) {
    // Handle request
    std::string path = getPath(request);

    if (isStaticRequest(request)) {
        return handelStaticRequest(request);
    } else {
        return handleDynamicRequest(request);
    }
}
