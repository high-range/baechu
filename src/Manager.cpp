#include "Manager.hpp"

#include "Configuration.hpp"
#include "Request.hpp"
#include "RequestData.hpp"
#include "Response.hpp"
#include "Worker.hpp"

std::string Manager::run(const std::string requestMessage,
                         const Configuration& configuration) {
    try {
        RequestData requestData;
        std::vector<std::pair<int, std::string> > responseMessageSource;

        Request::MessageParse(requestMessage, requestData, configuration);
        // responseMessageSource = Worker::run(requestData);
        // Worker가 status code 와 body 를 어떻게 반환할 것인지 미정.
        // 일단 std::vector<std::pair<int, std::string>> 를 반환한다고 가정.

        // 필요하다면 semantic 처리하는 함수 구현

        return (Response::MessageGenerate(responseMessageSource));
    } catch (std::vector<std::pair<int, std::string> >& responseMessageSource) {
        return (Response::MessageGenerate(responseMessageSource));
    }
}
