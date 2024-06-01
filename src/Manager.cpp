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

        Request::MessageParse(requestMessage, requestData, configuration);
        // responseMessageSource = Worker::run(requestData);
        return (Response::MessageGenerate(responseMessageSource));
    } catch (std::vector<std::pair<int, std::string> >& responseMessageSource) {
        return (Response::MessageGenerate(responseMessageSource));
    }
}
