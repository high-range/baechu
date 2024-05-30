#include "Worker.hpp"

#include <sys/stat.h>

#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "Response.hpp"

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

std::string generate_unique_filename() {
    // Get the current time since epoch
    std::time_t now = std::time(0);

    std::srand(std::time(0));
    int random_number =
        std::rand() % 9000 + 1;  // Generate a random number between 1 and 9000

    // Combine time and random number to create a unique filename
    std::string filename = "saved_file_" + std::to_string(now) + "_" +
                           std::to_string(random_number) + ".html";

    return filename;
}

bool Worker::saveFile(const std::string& dir, const std::string& content) {
    std::string filename = generate_unique_filename();
    std::string path = dir + "/" + filename;
    std::ofstream file(path);
    if (file.is_open()) {
        file << content;
        file.close();
        return true;
    } else {
        std::cerr << "Error opening file for writing" << std::endl;
        return false;
    }
}

void Worker::doPost(Request request) {
    std::string dir = getPath(request);
    std::string content = request.getBody();

    if (saveFile(dir, content)) {
        return;  // Return 201 Created if successful
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
        // return handleDynamicRequest(request);
    }
}
