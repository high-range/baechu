#include "Manager.hpp"

#include "Configuration.hpp"
#include "Request.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Worker.hpp"

std::string Manager::run(std::string requestMessage, RequestData requestData) {
    try {
        if (requestMessage.empty()) {
            throw ResponseData(400);
        }

        // TODO: have to add serverAddr and clientAddr to requestData
        (void)serverAddr;
        (void)clientAddr;

        Request::parseMessage(requestMessage, requestData);
        ResponseData responseData = Worker(requestData).handleRequest();

        return (Response::messageGenerate(responseData));
    } catch (ResponseData& responseData) {
        return (Response::messageGenerate(responseData));
    }
}
