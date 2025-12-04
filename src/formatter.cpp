#include "xlog/formatter.hpp"
#include "xlog/log_level.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace xlog {

std::string Formatter::format(const std::string& logger_name, LogLevel level, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm buf;
    localtime_r(&t, &buf);
    std::stringstream ss;
    ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    ss << " [" << to_string(level) << "] ";
    ss << logger_name << ": " << message;
    return ss.str();
}

}
