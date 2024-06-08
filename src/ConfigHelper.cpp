#include "Configuration.hpp"

const std::set<std::string> valid_block_names = {
    "http",   "server", "location",     "upstream",     "events",
    "stream", "mail",   "if",           "limit_except", "map",
    "types",  "geo",    "split_clients"};

const std::set<std::string> valid_directive_keys = {"accept_mutex",
                                                    "access_log",
                                                    "add_header",
                                                    "allow",
                                                    "deny",
                                                    "error_log",
                                                    "error_page",
                                                    "listen",
                                                    "root",
                                                    "index",
                                                    "methods",
                                                    "CGI",
                                                    "php"
                                                    "client_max_body_size",
                                                    "keepalive_timeout",
                                                    "proxy_pass",
                                                    "rewrite",
                                                    "sendfile",
                                                    "ssl_certificate",
                                                    "ssl_certificate_key",
                                                    "proxy_set_header",
                                                    "fastcgi_pass",
                                                    "include",
                                                    "default_type",
                                                    "log_format",
                                                    "server_name",
                                                    "worker_processes",
                                                    "user",
                                                    "pid",
                                                    "worker_connections",
                                                    "charset"};

bool isValidLocationBlockName(const std::string& name) {
    std::string path = name.substr(8);  // "location" 다음의 경로 추출
    return !path.empty();  // 경로가 비어있지 않은지 확인
}

bool isValidBlockName(const std::string& name) {
    if (name.find("location") == 0) {
        return isValidLocationBlockName(name);
    }
    return valid_block_names.find(name) != valid_block_names.end();
}

bool isValidDirectiveKey(const std::string& key) {
    return valid_directive_keys.find(key) != valid_directive_keys.end();
}

// 앞,뒤 tap,space 없애주는 함수
std::string trim(const std::string& str) {
    size_t not_tap_first = str.find_first_not_of(
        '\t');  // 문자열 시작에서 첫 번째 tap이 아닌 문자의 위치 찾기
    size_t not_space_first = str.find_first_not_of(
        ' ');  // 문자열 시작에서 첫 번째 space가 아닌 문자의 위치 찾기
    if (std::string::npos == not_tap_first ||
        std::string::npos == not_space_first)  // 문자열이 모두 공백인 경우
        return "";

    size_t not_tap_last = str.find_last_not_of(
        '\t');  // 문자열 끝에서 마지막 tap이 아닌 문자의 위치 찾기
    size_t not_space_last = str.find_last_not_of(
        ' ');  // 문자열 끝에서 마지막 space이 아닌 문자의 위치 찾기

    size_t first = std::max(not_tap_first, not_space_first);
    size_t last = std::min(not_tap_last, not_space_last);

    return str.substr(first,
                      (last - first + 1));  // 시작과 끝 사이의 부분 문자열 반환
}
