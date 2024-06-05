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
        std::vector<std::pair<int, std::string> > responseMessageSource;

        Request::messageParse(requestMessage, requestData, configuration);
        // responseMessageSource = Worker::run(requestData);

        return (Response::messageGenerate(responseMessageSource));
    } catch (std::vector<std::pair<int, std::string> >& responseMessageSource) {
        return (Response::messageGenerate(responseMessageSource));
    }
}
