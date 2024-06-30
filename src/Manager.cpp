#include "Manager.hpp"

#include <iostream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Worker.hpp"

void makeLog(std::string requestMessage) {
    std::ofstream logFile;
    logFile.open("log.txt", std::ios::app);
    logFile << requestMessage << std::endl;
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

std::string Manager::run(std::string requestMessage, RequestData requestData) {
    try {
        if (requestMessage.empty()) {
            throw ResponseData(400);
        }

        makeLog(requestMessage);

        // std::cout << "Request message logged" << std::endl;

        Request::parseMessage(requestMessage, requestData);

        // std::cout << "Request parsed" << std::endl;
        // showRequestData(requestData);

        Worker worker = Worker(requestData);
        ResponseData responseData = worker.handleRequest();
        responseData = worker.redirectOrUse(responseData);
        return (Response::messageGenerate(responseData));
    } catch (ResponseData& responseData) {
        responseData = Worker(requestData).redirectOrUse(responseData);
        return (Response::messageGenerate(responseData));
    }
}
