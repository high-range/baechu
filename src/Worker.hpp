#pragma once

#include <iostream>

#include "Request.hpp"
#include "Response.hpp"

class Worker {
  public:
    Response handleRequest(Request request);

  private:
    std::string getPath(Request request);
    bool isStaticRequest(Request request);
    Response handelStaticRequest(Request request);
    Response handleDynamicRequest(Request request);

    void doGet(Request request);

    void doPost(Request request);
    bool saveFile(const std::string& path, const std::string& content);

    void doDelete(Request request);
};
