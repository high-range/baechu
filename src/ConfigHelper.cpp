#include "Configuration.hpp"

// 앞,뒤 tap,space 없애주는 함수
std::string trim(const std::string& str) {
    size_t not_tap_first = str.find_first_not_of('\t'); // 문자열 시작에서 첫 번째 tap이 아닌 문자의 위치 찾기
	size_t not_space_first = str.find_first_not_of(' '); // 문자열 시작에서 첫 번째 space가 아닌 문자의 위치 찾기
    if (std::string::npos == not_tap_first || std::string::npos == not_space_first) // 문자열이 모두 공백인 경우
        return "";
		
    size_t not_tap_last = str.find_last_not_of('\t'); // 문자열 끝에서 마지막 tap이 아닌 문자의 위치 찾기
	size_t not_space_last = str.find_last_not_of(' '); // 문자열 끝에서 마지막 space이 아닌 문자의 위치 찾기

	size_t first = std::max(not_tap_first, not_space_first);
	size_t last = std::min(not_tap_last, not_space_last);

    return str.substr(first, (last - first + 1)); // 시작과 끝 사이의 부분 문자열 반환
}