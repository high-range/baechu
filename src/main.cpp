#include <iostream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "Worker.hpp"

int main() {
    Request request("DELETE");  // POST, GET1, GET2, DELETE
    Worker worker;

    Response response = worker.handleRequest(request);
    std::cout << response.toString() << std::endl;
    return 0;
}
