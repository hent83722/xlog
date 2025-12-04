#pragma once
#include <string>
#include "log_level.hpp"

namespace xlog {

class Formatter {
public:
    std::string format(const std::string& logger_name, LogLevel level, const std::string& message);
};

}
