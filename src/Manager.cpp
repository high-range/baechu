#include "Manager.hpp"

#include <iostream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Worker.hpp"

void makeLog(std::string message, std::string type) {
    std::ofstream logFile;
    logFile.open("log.txt", std::ios::app);
    logFile << type << message << std::endl;
    logFile.close();
}

void showRequestData(RequestData& requestData) {
    std::map<std::string, std::string> header = requestData.getHeader();
    std::map<std::string, std::string>::iterator begin = header.begin();
    std::map<std::string, std::string>::iterator end = header.end();
    std::cout << "\n-----------------------------\n";
    std::cout << "Client IP: " << requestData.getClientIP() << std::endl;
    std::cout << "Client Port: " << requestData.getClientPort() << std::endl;
    std::cout << "Server IP: " << requestData.getServerIP() << std::endl;
    std::cout << "Server Port: " << requestData.getServerPort() << std::endl;
    std::cout << "\n-----------------------------\n";
    std::cout << "Method: " << requestData.getMethod() << std::endl;
    std::cout << "RequestTarget: " << requestData.getRequestTarget()
              << std::endl;
    std::cout << "Path: " << requestData.getPath() << std::endl;
    std::cout << "Query: " << requestData.getQuery() << std::endl;
    std::cout << "Version: " << requestData.getVersion() << std::endl;
    std::cout << "<Header>" << std::endl;
    for (; begin != end; begin++) {
        std::cout << begin->first << ": " << begin->second << std::endl;
    }
    std::cout << "Body: " << requestData.getBody() << std::endl;
}

// TODO : Remove 주석 처리된 코드
std::string Manager::run(RequestData requestData, ResponseData responseData) {
    Worker worker = Worker(requestData);

    // showRequestData(requestData);
    responseData = worker.resolveErrorPage(responseData);
    // makeLog(Response::messageGenerate(responseData),
    //         "[Response]\n\n");  // TODO: Remove this line
    return (Response::messageGenerate(responseData));
}

std::string Manager::run(RequestData requestData) {
    Worker worker = Worker(requestData);
    ResponseData responseData = worker.handleRequest();

    // showRequestData(requestData);
    responseData = worker.resolveErrorPage(responseData);
    // makeLog(Response::messageGenerate(responseData),
    //         "[Response]\n\n");  // TODO: Remove this line
    return (Response::messageGenerate(responseData));
}
