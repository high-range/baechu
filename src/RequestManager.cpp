#include "RequestManager.hpp"

#include "Parser.hpp"

std::vector<std::pair<int, std::string> > RequestManager::parse(
    std::string requestMessage, RequestData& requestData) {
    try {
        Parser::startLineParse(requestMessage, requestData);
        Parser::headerParse(requestMessage, requestData);
        // Parser::bodyParse(requestMessage, requestData);
        return std::vector<std::pair<int, std::string> >(
            1, std::make_pair(0, "OK"));
    } catch (const std::vector<std::pair<int, std::string> >& statusData) {
        return statusData;
    }
}
