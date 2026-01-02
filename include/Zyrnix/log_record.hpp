#pragma once
#include "log_level.hpp"
#include <string>
#include <chrono>
#include <unordered_map>

namespace Zyrnix {

struct LogRecord {
    std::string logger_name;
    LogLevel level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> fields;
    
    bool has_field(const std::string& key) const {
        return fields.find(key) != fields.end();
    }
    
    std::string get_field(const std::string& key) const {
        auto it = fields.find(key);
        return (it != fields.end()) ? it->second : "";
    }
};

}
