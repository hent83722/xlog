#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include "log_level.hpp"

namespace Zyrnix {

class Logger;

/**
 * @brief Configuration for a single logger
 */
struct LoggerConfig {
    std::string name;
    LogLevel level = LogLevel::Info;
    bool async = false;
    std::vector<std::string> sinks;
    std::map<std::string, std::string> sink_params;

    // Redaction configuration (v1.1.3)
    // These are stored as raw strings and interpreted by ConfigLoader
    // to configure Logger redaction behaviour.
    std::string redact_substrings;
    std::string redact_regexes;
    std::string redact_presets;
    bool redact_cloud_only = false;
};

/**
 * @brief Configuration loader for Zyrnix
 * 
 * Supports JSON format configuration files to set up loggers
 * without recompiling. This allows dynamic configuration in production.
 * 
 * Example JSON format:
 * {
 *   "loggers": [
 *     {
 *       "name": "app",
 *       "level": "info",
 *       "async": true,
 *       "sinks": [
 *         {"type": "stdout"},
 *         {"type": "file", "path": "/var/log/app.log"},
 *         {"type": "rotating", "path": "app.log", "max_size": 10485760, "max_files": 5}
 *       ]
 *     }
 *   ]
 * }
 */
class ConfigLoader {
public:
    /**
     * @brief Load configuration from JSON file
     * @param path Path to JSON config file
     * @return true if loaded successfully
     */
    static bool load_from_json(const std::string& path);
    
    /**
     * @brief Load configuration from JSON string
     * @param json_str JSON string content
     * @return true if parsed successfully
     */
    static bool load_from_json_string(const std::string& json_str);
    
    /**
     * @brief Get all configured loggers
     * @return Vector of logger configurations
     */
    static std::vector<LoggerConfig> get_logger_configs();
    
    /**
     * @brief Create loggers from loaded configuration
     * @return Map of logger name to Logger instance
     */
    static std::map<std::string, std::shared_ptr<Logger>> create_loggers();
    
    /**
     * @brief Clear all loaded configurations
     */
    static void clear();

    /**
     * @brief Get the last configuration parse error message (v1.1.3)
     * @return Human-readable error string, or empty if no error
     */
    static std::string get_last_error();

private:
    static std::vector<LoggerConfig> configs_;
    
    static LogLevel parse_log_level(const std::string& level);
    static bool parse_json_internal(const std::string& content);
};

class Config {
public:
    Config();
};

}
