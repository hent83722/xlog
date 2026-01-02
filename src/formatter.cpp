#include "Zyrnix/formatter.hpp"
#include "Zyrnix/log_level.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Zyrnix {

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

std::string Formatter::redact(const std::string& message, const std::vector<std::string>& patterns) {
    std::string redacted = message;
    for (const auto& pat : patterns) {
        size_t pos = 0;
        while ((pos = redacted.find(pat, pos)) != std::string::npos) {
            redacted.replace(pos, pat.length(), std::string(pat.length(), '*'));
            pos += pat.length();
        }
    }
    return redacted;
}

}
