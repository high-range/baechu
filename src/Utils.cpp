#include "Utils.hpp"

std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

int stringToInteger(const std::string& str) {
    std::stringstream ss(str);
    int num;

    ss >> num;
    return num;
}
