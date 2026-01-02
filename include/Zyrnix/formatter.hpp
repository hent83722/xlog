#pragma once
#include <string>
#include <vector>
#include "log_level.hpp"

namespace Zyrnix {

class Formatter {
public:
    std::string format(const std::string& logger_name, LogLevel level, const std::string& message);
    static std::string redact(const std::string& message, const std::vector<std::string>& patterns);
};

}
