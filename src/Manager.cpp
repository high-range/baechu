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

std::string Manager::run(std::string requestMessage, RequestData requestData) {
    try {
        if (requestMessage.empty()) {
            throw ResponseData(400);
        }

        makeLog(requestMessage);

        // std::cout << "Request message logged" << std::endl;

        Request::parseMessage(requestMessage, requestData);

        // std::cout << "Request parsed" << std::endl;

        ResponseData responseData = Worker(requestData).handleRequest();
        return (Response::messageGenerate(responseData));
    } catch (ResponseData& responseData) {
        return (Response::messageGenerate(responseData));
    }
}
