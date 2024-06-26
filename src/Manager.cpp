#include "Manager.hpp"

#include "Configuration.hpp"
#include "Request.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Worker.hpp"

std::string Manager::run(std::string requestMessage,
                         const Configuration& configuration) {
    try {
        RequestData requestData;
        if (requestMessage.empty()) {
            throw ResponseData(400);
        }

        Request::messageParse(requestMessage, requestData, configuration);
        ResponseData responseData = Worker(requestData).handleRequest();

        return (Response::messageGenerate(responseData));
    } catch (ResponseData& responseData) {
        return (Response::messageGenerate(responseData));
    }
}
