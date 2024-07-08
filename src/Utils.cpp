#include "Utils.hpp"

std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
