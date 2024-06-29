#include <iostream>

#include "Configuration.hpp"

const std::set<std::string> createValidBlockNames() {
    std::set<std::string> s;
    s.insert("http");
    s.insert("server");
    s.insert("location");
    s.insert("events");
    return s;
}

const std::set<std::string> createValidDirectiveKeys() {
    std::set<std::string> s;
    s.insert("allow");
    s.insert("deny");
    s.insert("error_log");
    s.insert("error_page");
    s.insert("listen");
    s.insert("root");
    s.insert("index");
    s.insert("limit_except");
    s.insert("client_max_body_size");
    s.insert("rewrite");
    s.insert("default_type");
    s.insert("log_format");
    s.insert("server_name");
    s.insert("worker_processes");
    s.insert("user");
    s.insert("pid");
    s.insert("worker_connections");
    s.insert("charset");
    s.insert("ssl_protocols");
    s.insert("ssl_prefer_server_ciphers");
    s.insert("access_log");
    s.insert("include");
    s.insert("sendfile");
    s.insert("keepalive_timeout");
    s.insert("return");
    return s;
}

const std::set<std::string> createValidMainDirectives() {
    std::set<std::string> s;
    s.insert("env");
    s.insert("error_log");
    s.insert("pid");
    s.insert("worker_processes");
    s.insert("http");
    s.insert("events");
    s.insert("user");
    return s;
}

const std::set<std::string> createValidHttpDirectives() {
    std::set<std::string> s;
    s.insert("error_log");
    s.insert("client_body_buffer_size");
    s.insert("client_max_body_size");
    s.insert("default_type");
    s.insert("error_page");
    s.insert("server");
    s.insert("root");
    s.insert("type");
    s.insert("include");
    s.insert("sendfile");
    s.insert("keepalive_timeout");
    s.insert("sendfile");
    return s;
}

const std::set<std::string> createValidServerDirectives() {
    std::set<std::string> s;
    s.insert("client_body_buffer_size");
    s.insert("client_max_body_size");
    s.insert("default_type");
    s.insert("error_page");
    s.insert("location");
    s.insert("root");
    s.insert("server_name");
    s.insert("type");
    s.insert("listen");
    s.insert("error_log");
    return s;
}

const std::set<std::string> createValidLocationDirectives() {
    std::set<std::string> s;
    s.insert("error_log");
    s.insert("client_body_buffer_size");
    s.insert("client_max_body_size");
    s.insert("default_type");
    s.insert("error_page");
    s.insert("root");
    s.insert("alias");
    s.insert("type");
    s.insert("index");
    s.insert("limit_except");
    s.insert("return");
    return s;
}

const std::set<std::string> createValidEventsDirectives() {
    std::set<std::string> s;
    s.insert("use");
    s.insert("worker_connections");
    return s;
}

const std::set<std::string> valid_block_names = createValidBlockNames();
const std::set<std::string> valid_directive_keys = createValidDirectiveKeys();
const std::set<std::string> valid_main_directives = createValidMainDirectives();
const std::set<std::string> valid_http_directives = createValidHttpDirectives();
const std::set<std::string> valid_server_directives =
    createValidServerDirectives();
const std::set<std::string> valid_location_directives =
    createValidLocationDirectives();
const std::set<std::string> valid_events_directives =
    createValidEventsDirectives();

bool isValidLocationBlockName(const std::string& name) {
    // "location" 다음의 경로 추출
    std::string path = name.substr(8);
    // 경로가 비어있지 않은지 확인
    return !path.empty();
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

bool isValidKeyInBlock(const std::string& block_name, const std::string& key) {
    if (block_name == "main") {
        return valid_main_directives.find(key) != valid_main_directives.end();
    }
    if (block_name == "http") {
        return valid_http_directives.find(key) != valid_http_directives.end();
    }
    if (block_name == "server") {
        if (key.substr(0, 9) == "location ") {
            return true;
        }
        return valid_server_directives.find(key) !=
               valid_server_directives.end();
    }
    if (block_name.substr(0, 9) == "location ") {
        return valid_location_directives.find(key) !=
               valid_location_directives.end();
    }
    if (block_name == "events") {
        return valid_events_directives.find(key) !=
               valid_events_directives.end();
    }
    return true;
}

// 앞,뒤 tap,space 없애주는 함수
std::string trim(const std::string& str) {
    // 문자열 시작에서 첫 번째 공백이나 탭이 아닌 문자의 위치 찾기
    size_t not_space_or_tab_first = str.find_first_not_of(" \t");
    // 문자열 끝에서 첫 번째 공백이나 탭이 아닌 문자의 위치 찾기
    size_t not_space_or_tab_last = str.find_last_not_of(" \t");

    // 공백이나 탭만으로 이루어진 문자열인지 확인
    if (not_space_or_tab_first == std::string::npos) {
        return "";
    }

    // 공백이나 탭이 아닌 문자로 양쪽을 잘라낸 새로운 문자열 반환
    return str.substr(not_space_or_tab_first,
                      not_space_or_tab_last - not_space_or_tab_first + 1);
}

int countMatchingPrefixLength(const std::string& location,
                              const std::string& request_location) {
    int count = 0;
    size_t i = 0;
    for (i = 0; i < location.length() && i < request_location.length(); i++) {
        if (location[i] != request_location[i]) {
            break;
        }
        count++;
    }
    return count;
}