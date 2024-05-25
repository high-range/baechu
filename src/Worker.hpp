#pragma once

#include <iostream>

#include "Request.hpp"
#include "Response.hpp"
class Worker {
  public:
    // Worker();
    // ~Worker();
    // Worker(const Worker& copy);
    // Worker& operator=(const Worker& copy);
    Response handleRequest(Request request);

  private:
    std::string getPath(Request request);
    bool isStaticRequest(Request request);
    Response handelStaticRequest(Request request);
    Response handleDynamicRequest(Request request);
    void doGet(Request request);
    void doPost(Request request);
    void doDelete(Request request);
};
