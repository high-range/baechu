#include <iostream>

#include "Configuration.hpp"
#include "Request.hpp"
#include "Worker.hpp"

/*
📚 Request message:
POST /test HTTP/1.1
Host: localhost:8080
Content-Type: text/html
Content-Length: 27

<html><body>test</body></html>

📚 Server configuration:
server {
        listen       8080;
        server_name  localhost;

        location / {
            root   /Users/leesiha/42/nginxTest;
            index  index.html index.htm;
        }

        location /test {
            root   /Users/leesiha/42/nginxTest;
            index  index.html index.htm;
        }
    }

✅ Expected output:

1️⃣ 서버가 올바르게 설정되어 있고 요청에 맞는 파일이 존재할 경우
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: [파일 내용의 길이]

[index.html의 내용]

2️⃣ 파일이 존재하지 않거나 요청 방식(POST)이 허용되지 않을 경우

HTTP/1.1 404 Not Found (또는 405 Method Not Allowed)
Content-Type: text/html
Content-Length: [오류 페이지의 길이]

<html><body>404 Not Found</body></html> (또는 405 Method Not Allowed)

*/

std::string requestMSG =
    "POST /test HTTP/1.1\r\nHost: localhost:8080\r\nContent-Type: "
    "text/html\r\nContent-Length: 27\r\n\r\n<html><body>test</body></html>";

std::string responseMSG =
    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 27\r\n\r\n"
    "<html><body>test</body></html>";

std::string configMSG =
    "server {\n"
    "        listen       8080;\n"
    "        server_name  localhost;\n"
    "\n"
    "        location / {\n"
    "            root   /Users/leesiha/42/nginxTest;\n"
    "            index  index.html index.htm;\n"
    "        }\n"
    "\n"
    "        location /test {\n"
    "            root   /Users/leesiha/42/nginxTest;\n"
    "            index  index.html index.htm;\n"
    "        }\n"
    "    }";

int main() {
    Request request(requestMSG);
    Configuration config(configMSG);
    Worker worker;
    worker.handleRequest(request);
    return 0;
}

// ./webserv가 성공적으로 수행됐다면, /Users/leesiha/42/nginxTest/test 경로에
// 파일이 생성되었을 것입니다. 이 파일을 열어서 내용을 확인해보세요.
