#include "xlog/util.hpp"

namespace xlog {

std::string trim(const std::string& s) {
    auto b = s.find_first_not_of(" \t\n\r");
    auto e = s.find_last_not_of(" \t\n\r");
    if (b == std::string::npos) return "";
    return s.substr(b, e - b + 1);
}

}
