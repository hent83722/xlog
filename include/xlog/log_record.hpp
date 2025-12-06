#pragma once
#include "log_level.hpp"
#include <string>
#include <chrono>

namespace xlog {

struct LogRecord {
    std::string logger_name;
    LogLevel level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};

}
