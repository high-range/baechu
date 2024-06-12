#include "Configuration.hpp"

const std::set<std::string> createValidBlockNames() {
    std::set<std::string> s;
    s.insert("http");
    s.insert("server");
    s.insert("location");
    s.insert("upstream");
    s.insert("events");
    s.insert("stream");
    s.insert("mail");
    s.insert("if");
    s.insert("limit_except");
    s.insert("map");
    s.insert("types");
    s.insert("geo");
    s.insert("split_clients");
    return s;
}

const std::set<std::string> createValidDirectiveKeys() {
    std::set<std::string> s;
    s.insert("accept_mutex");
    s.insert("access_log");
    s.insert("add_header");
    s.insert("allow");
    s.insert("deny");
    s.insert("error_log");
    s.insert("error_page");
    s.insert("listen");
    s.insert("root");
    s.insert("index");
    s.insert("methods");
    s.insert("CGI");
    s.insert("php");
    s.insert("client_max_body_size");
    s.insert("keepalive_timeout");
    s.insert("proxy_pass");
    s.insert("rewrite");
    s.insert("sendfile");
    s.insert("ssl_certificate");
    s.insert("ssl_certificate_key");
    s.insert("proxy_set_header");
    s.insert("fastcgi_pass");
    s.insert("include");
    s.insert("default_type");
    s.insert("log_format");
    s.insert("server_name");
    s.insert("worker_processes");
    s.insert("user");
    s.insert("pid");
    s.insert("worker_connections");
    s.insert("charset");
    return s;
}

const std::set<std::string> valid_block_names = createValidBlockNames();
const std::set<std::string> valid_directive_keys = createValidDirectiveKeys();

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
