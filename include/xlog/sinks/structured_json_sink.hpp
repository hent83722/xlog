#pragma once
#include "../log_sink.hpp"
#include "../log_level.hpp"
#include <string>
#include <map>
#include <fstream>
#include <mutex>
#include <memory>

namespace xlog {

/**
 * StructuredJsonSink outputs logs as JSON objects with support for custom fields and context.
 * Useful for cloud platforms, log aggregators (ELK, Datadog, Splunk, etc.)
 * 
 * Example output:
 * {
 *   "timestamp": "2025-12-07T16:46:36.123Z",
 *   "level": "INFO",
 *   "logger": "http_server",
 *   "message": "User logged in",
 *   "user_id": "12345",
 *   "ip": "192.168.1.100"
 * }
 */
class StructuredJsonSink : public LogSink {
public:
    explicit StructuredJsonSink(const std::string& filename);
    ~StructuredJsonSink();
    
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;
    

    void set_context(const std::string& key, const std::string& value);

    void log_with_fields(const std::string& logger_name, LogLevel level, 
                         const std::string& message,
                         const std::map<std::string, std::string>& fields);
    

    void clear_context();

private:
    std::string filename;
    std::map<std::string, std::string> global_context;
    std::ofstream file;
    std::mutex mtx;
    
    std::string build_json(const std::string& logger_name, LogLevel level,
                          const std::string& message,
                          const std::map<std::string, std::string>& fields);
};

}
