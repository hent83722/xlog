#include "xlog/logger.hpp"
#include "xlog/log_sink.hpp"
#include "xlog/log_filter.hpp"
#include "xlog/sinks/stdout_sink.hpp"
#include "xlog/async/async_logger.hpp"
#include "xlog/log_health.hpp"
#include <mutex>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace xlog {

Logger::Logger(std::string n) 
    : name(std::move(n)), min_level_(LogLevel::Trace) {
    temp_level_.active = false;
}

void Logger::add_sink(LogSinkPtr sink) {
    std::lock_guard<std::mutex> lock(mtx);
    sinks.push_back(std::move(sink));
}

void Logger::clear_sinks() {
    std::lock_guard<std::mutex> lock(mtx);
    sinks.clear();
    sink_level_overrides_.clear();
    sink_level_overrides_by_name_.clear();
}

void Logger::set_level(LogLevel level) {
    min_level_.store(level, std::memory_order_relaxed);
}

LogLevel Logger::get_level() const {
    return min_level_.load(std::memory_order_relaxed);
}

void Logger::set_level_dynamic(LogLevel level) {
    set_level_dynamic(level, "");
}

void Logger::set_level_dynamic(LogLevel level, const std::string& reason) {
    check_temporary_level_expiry();
    
    LogLevel old_level = min_level_.exchange(level, std::memory_order_release);
    
    if (old_level != level) {
        std::lock_guard<std::mutex> lock(mtx);
        record_level_change(old_level, level, reason);
        
        for (const auto& callback : level_change_callbacks_) {
            callback(old_level, level);
        }
    }
}

void Logger::register_level_change_callback(LogLevelChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mtx);
    level_change_callbacks_.push_back(std::move(callback));
}

void Logger::clear_level_change_callbacks() {
    std::lock_guard<std::mutex> lock(mtx);
    level_change_callbacks_.clear();
}

void Logger::set_sink_level(size_t sink_index, LogLevel level) {
    std::lock_guard<std::mutex> lock(mtx);
    if (sink_index < sinks.size()) {
        sink_level_overrides_[sink_index] = level;
    }
}

void Logger::set_sink_level(const std::string& sink_name, LogLevel level) {
    std::lock_guard<std::mutex> lock(mtx);
    sink_level_overrides_by_name_[sink_name] = level;
}

void Logger::clear_sink_level_overrides() {
    std::lock_guard<std::mutex> lock(mtx);
    sink_level_overrides_.clear();
    sink_level_overrides_by_name_.clear();
}

void Logger::record_level_change(LogLevel old_level, LogLevel new_level, const std::string& reason) {
    LevelChangeEntry entry;
    entry.old_level = old_level;
    entry.new_level = new_level;
    entry.timestamp = std::chrono::system_clock::now();
    entry.reason = reason;
    
    level_history_.push_back(entry);
    
    // Trim history if needed
    while (level_history_.size() > max_history_entries_) {
        level_history_.pop_front();
    }
}

std::vector<LevelChangeEntry> Logger::get_level_history(size_t max_entries) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mtx));
    
    std::vector<LevelChangeEntry> result;
    size_t count = std::min(max_entries, level_history_.size());
    
    // Return most recent entries
    auto start = level_history_.end() - count;
    result.assign(start, level_history_.end());
    
    return result;
}

void Logger::clear_level_history() {
    std::lock_guard<std::mutex> lock(mtx);
    level_history_.clear();
}

void Logger::set_max_history_entries(size_t max_entries) {
    std::lock_guard<std::mutex> lock(mtx);
    max_history_entries_ = max_entries;
    
    // Trim if needed
    while (level_history_.size() > max_history_entries_) {
        level_history_.pop_front();
    }
}

void Logger::set_level_temporary(LogLevel level, std::chrono::seconds duration, 
                                 const std::string& reason) {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (!temp_level_.active) {
        temp_level_.original_level = min_level_.load(std::memory_order_acquire);
    }
    
    temp_level_.revert_time = std::chrono::system_clock::now() + duration;
    temp_level_.active = true;
    
    LogLevel old_level = min_level_.exchange(level, std::memory_order_release);
    
    std::string full_reason = reason.empty() ? 
        "Temporary level change for " + std::to_string(duration.count()) + "s" :
        reason + " (temporary, " + std::to_string(duration.count()) + "s)";
    
    record_level_change(old_level, level, full_reason);
    
    for (const auto& callback : level_change_callbacks_) {
        callback(old_level, level);
    }
}

void Logger::cancel_temporary_level() {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (temp_level_.active) {
        LogLevel current = min_level_.load(std::memory_order_acquire);
        LogLevel original = temp_level_.original_level;
        
        min_level_.store(original, std::memory_order_release);
        temp_level_.active = false;
        
        record_level_change(current, original, "Temporary level cancelled");
        
        for (const auto& callback : level_change_callbacks_) {
            callback(current, original);
        }
    }
}

bool Logger::has_temporary_level() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mtx));
    return temp_level_.active;
}

std::chrono::seconds Logger::remaining_temporary_duration() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mtx));
    
    if (!temp_level_.active) {
        return std::chrono::seconds(0);
    }
    
    auto now = std::chrono::system_clock::now();
    if (now >= temp_level_.revert_time) {
        return std::chrono::seconds(0);
    }
    
    return std::chrono::duration_cast<std::chrono::seconds>(temp_level_.revert_time - now);
}

void Logger::check_temporary_level_expiry() {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (temp_level_.active) {
        auto now = std::chrono::system_clock::now();
        if (now >= temp_level_.revert_time) {
            LogLevel current = min_level_.load(std::memory_order_acquire);
            LogLevel original = temp_level_.original_level;
            
            min_level_.store(original, std::memory_order_release);
            temp_level_.active = false;
            
            record_level_change(current, original, "Temporary level expired");
            
            for (const auto& callback : level_change_callbacks_) {
                callback(current, original);
            }
        }
    }
}

void Logger::add_filter(std::shared_ptr<LogFilter> filter) {
    std::lock_guard<std::mutex> lock(mtx);
    filters_.push_back(std::move(filter));
}

void Logger::clear_filters() {
    std::lock_guard<std::mutex> lock(mtx);
    filters_.clear();
    filter_func_ = nullptr;
}

void Logger::set_filter_func(std::function<bool(const LogRecord&)> func) {
    std::lock_guard<std::mutex> lock(mtx);
    filter_func_ = std::move(func);
}

bool Logger::should_log(const LogRecord& record) const {
    if (record.level < min_level_.load(std::memory_order_acquire)) {
        return false;
    }
    
    if (filter_func_ && !filter_func_(record)) {
        return false;
    }
    
    for (const auto& filter : filters_) {
        if (!filter->should_log(record)) {
            return false;
        }
    }
    
    return true;
}

void Logger::log(LogLevel level, const std::string& message) {
    check_temporary_level_expiry();
    
    LogRecord record;
    record.logger_name = name;
    record.level = level;
    record.message = message;
    record.timestamp = std::chrono::system_clock::now();
    
    std::lock_guard<std::mutex> lock(mtx);
    
    if (!should_log(record)) {
        return;
    }
    
    for (size_t i = 0; i < sinks.size(); ++i) {
        auto override_it = sink_level_overrides_.find(i);
        if (override_it != sink_level_overrides_.end()) {
            if (level < override_it->second) {
                continue;
            }
        }
        
        sinks[i]->log(name, level, message);
    }
}

void Logger::trace(const std::string& msg) { log(LogLevel::Trace, msg); }
void Logger::debug(const std::string& msg) { log(LogLevel::Debug, msg); }
void Logger::info(const std::string& msg) { log(LogLevel::Info, msg); }
void Logger::warn(const std::string& msg) { log(LogLevel::Warn, msg); }
void Logger::error(const std::string& msg) { log(LogLevel::Error, msg); }
void Logger::critical(const std::string& msg) { log(LogLevel::Critical, msg); }

std::shared_ptr<Logger> Logger::create_stdout_logger(const std::string& name) {
    auto logger = std::make_shared<Logger>(name);
    logger->add_sink(std::make_shared<StdoutSink>());
    
    HealthRegistry::auto_register(name, logger);
    
    return logger;
}

std::shared_ptr<Logger> Logger::create_async(const std::string& name) {
    auto logger = std::make_shared<Logger>(name);
    
    HealthRegistry::auto_register(name, logger);
    
    return logger;
}

std::string LogLevelControlResponse::to_json() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"success\": " << (success ? "true" : "false") << ",\n";
    oss << "  \"message\": \"" << message << "\",\n";
    oss << "  \"logger_name\": \"" << logger_name << "\",\n";
    oss << "  \"current_level\": \"";
    
    switch (current_level) {
        case LogLevel::Trace: oss << "trace"; break;
        case LogLevel::Debug: oss << "debug"; break;
        case LogLevel::Info: oss << "info"; break;
        case LogLevel::Warn: oss << "warn"; break;
        case LogLevel::Error: oss << "error"; break;
        case LogLevel::Critical: oss << "critical"; break;
        default: oss << "unknown"; break;
    }
    oss << "\"\n";
    oss << "}";
    return oss.str();
}

std::pair<bool, LogLevel> parse_log_level(const std::string& level_str) {
    std::string lower = level_str;
    std::transform(lower.begin(), lower.end(), lower.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    if (lower == "trace") return {true, LogLevel::Trace};
    if (lower == "debug") return {true, LogLevel::Debug};
    if (lower == "info") return {true, LogLevel::Info};
    if (lower == "warn" || lower == "warning") return {true, LogLevel::Warn};
    if (lower == "error") return {true, LogLevel::Error};
    if (lower == "critical" || lower == "fatal") return {true, LogLevel::Critical};
    
    return {false, LogLevel::Info};
}

LogLevelControlResponse handle_level_change_request(
    std::shared_ptr<Logger> logger,
    const std::string& new_level_str,
    const std::string& reason,
    int duration_seconds) {
    
    LogLevelControlResponse response;
    response.logger_name = logger ? logger->name : "";
    
    if (!logger) {
        response.success = false;
        response.message = "Logger not found";
        response.current_level = LogLevel::Info;
        return response;
    }
    
    auto [valid, level] = parse_log_level(new_level_str);
    
    if (!valid) {
        response.success = false;
        response.message = "Invalid log level: " + new_level_str;
        response.current_level = logger->get_level();
        return response;
    }
    
    if (duration_seconds > 0) {
        logger->set_level_temporary(level, std::chrono::seconds(duration_seconds), reason);
        response.message = "Log level changed temporarily for " + 
                          std::to_string(duration_seconds) + " seconds";
    } else {
        logger->set_level_dynamic(level, reason);
        response.message = "Log level changed successfully";
    }
    
    response.success = true;
    response.current_level = logger->get_level();
    return response;
}

}
