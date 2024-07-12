#include "Manager.hpp"

#include <iostream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Worker.hpp"

// void showRequestData(RequestData& requestData) {
//     std::map<std::string, std::string> header = requestData.getHeader();
//     std::map<std::string, std::string>::iterator begin = header.begin();
//     std::map<std::string, std::string>::iterator end = header.end();
//     std::cout << "\n-----------------------------\n";
//     std::cout << "Client IP: " << requestData.getClientIP() << std::endl;
//     std::cout << "Client Port: " << requestData.getClientPort() << std::endl;
//     std::cout << "Server IP: " << requestData.getServerIP() << std::endl;
//     std::cout << "Server Port: " << requestData.getServerPort() << std::endl;
//     std::cout << "\n-----------------------------\n";
//     std::cout << "Method: " << requestData.getMethod() << std::endl;
//     std::cout << "RequestTarget: " << requestData.getRequestTarget()
//               << std::endl;
//     std::cout << "Path: " << requestData.getPath() << std::endl;
//     std::cout << "Query: " << requestData.getQuery() << std::endl;
//     std::cout << "Version: " << requestData.getVersion() << std::endl;
//     std::cout << "<Header>" << std::endl;
//     for (; begin != end; begin++) {
//         std::cout << begin->first << ": " << begin->second << std::endl;
//     }
//     std::cout << "Body: " << requestData.getBody() << std::endl;
//     std::cout << "--------------------------------\n";
// }

std::string Manager::run(RequestData requestData, ResponseData responseData) {
    // showRequestData(requestData);

    Worker worker = Worker(requestData);
    responseData = worker.resolveErrorPage(responseData);

    return (Response::messageGenerate(responseData));
}

std::string Manager::run(RequestData requestData) {
    // showRequestData(requestData);

    Worker worker = Worker(requestData);
    ResponseData responseData = worker.handleRequest();
    responseData = worker.resolveErrorPage(responseData);

    return (Response::messageGenerate(responseData));
}
