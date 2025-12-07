#include "xlog/sinks/structured_json_sink.hpp"
#include "xlog/log_level.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace xlog {

StructuredJsonSink::StructuredJsonSink(const std::string& fname)
    : filename(fname) {
    file.open(filename, std::ios::app);
}

StructuredJsonSink::~StructuredJsonSink() {
    if (file.is_open()) {
        file.close();
    }
}

std::string escape_json_string(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c < 32) {
                    std::ostringstream oss;
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                    result += oss.str();
                } else {
                    result += c;
                }
        }
    }
    return result;
}

std::string get_iso8601_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t_now), "%Y-%m-%dT%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
    return oss.str();
}

std::string StructuredJsonSink::build_json(const std::string& logger_name, LogLevel level,
                                           const std::string& message,
                                           const std::map<std::string, std::string>& fields) {
    std::ostringstream json;
    json << "{";
    
 
    json << "\"timestamp\":\"" << get_iso8601_timestamp() << "\",";
    
 
    json << "\"level\":\"" << to_string(level) << "\",";
    
 
    json << "\"logger\":\"" << escape_json_string(logger_name) << "\",";
    

    json << "\"message\":\"" << escape_json_string(message) << "\"";
    

    for (const auto& [key, value] : global_context) {
        json << ",\"" << escape_json_string(key) << "\":\"" << escape_json_string(value) << "\"";
    }
    

    for (const auto& [key, value] : fields) {
        json << ",\"" << escape_json_string(key) << "\":\"" << escape_json_string(value) << "\"";
    }
    
    json << "}";
    return json.str();
}

void StructuredJsonSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    std::map<std::string, std::string> empty_fields;
    log_with_fields(logger_name, level, message, empty_fields);
}

void StructuredJsonSink::log_with_fields(const std::string& logger_name, LogLevel level,
                                         const std::string& message,
                                         const std::map<std::string, std::string>& fields) {
    std::lock_guard<std::mutex> lock(mtx);
    if (file.is_open()) {
        std::string json_line = build_json(logger_name, level, message, fields);
        file << json_line << std::endl;
        file.flush();
    }
}

void StructuredJsonSink::set_context(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    global_context[key] = value;
}

void StructuredJsonSink::clear_context() {
    std::lock_guard<std::mutex> lock(mtx);
    global_context.clear();
}

}
