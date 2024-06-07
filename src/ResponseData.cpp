#include "ResponseData.hpp"

ResponseData::ResponseData(int statusCode, Headers headers, std::string body) {
    this->statusCode = statusCode;
    this->headers = headers;
    this->body = body;
}

ResponseData::ResponseData(int statusCode) {
    this->statusCode = statusCode;
    this->headers = Headers();
    this->body = "";
}

ResponseData::ResponseData(int statusCode, std::string body) {
    this->statusCode = statusCode;
    this->headers = Headers();
    this->body = body;
}

ResponseData::ResponseData(int statusCode, Headers headers) {
    this->statusCode = statusCode;
    this->headers = headers;
    this->body = "";
}

ResponseData::ResponseData() {
    this->statusCode = 200;
    this->headers = Headers();
    this->body = "";
}
