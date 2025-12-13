#pragma once
#include "xlog_features.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>
#include <chrono>
#include <deque>
#include <map>
#include "log_sink.hpp"
#include "log_level.hpp"
#include "log_record.hpp"

namespace xlog {

#ifndef XLOG_NO_FILTERS
class LogFilter;
#endif

struct LevelChangeEntry {
    LogLevel old_level;
    LogLevel new_level;
    std::chrono::system_clock::time_point timestamp;
    std::string reason;
};

struct TemporaryLevelChange {
    LogLevel original_level;
    std::chrono::system_clock::time_point revert_time;
    bool active;
};

class Logger {
public:
    explicit Logger(std::string name);

    void add_sink(LogSinkPtr sink);
    void clear_sinks();
    void log(LogLevel level, const std::string& message);

    void trace(const std::string& msg);
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void critical(const std::string& msg);
    
    void set_level(LogLevel level);
    LogLevel get_level() const;
    
    using LogLevelChangeCallback = std::function<void(LogLevel old_level, LogLevel new_level)>;
    void set_level_dynamic(LogLevel level);  
    void register_level_change_callback(LogLevelChangeCallback callback);
    void clear_level_change_callbacks();
    
    void set_level_dynamic(LogLevel level, const std::string& reason);
    
    void set_sink_level(size_t sink_index, LogLevel level);
    void set_sink_level(const std::string& sink_name, LogLevel level);
    void clear_sink_level_overrides();
    
    std::vector<LevelChangeEntry> get_level_history(size_t max_entries = 100) const;
    void clear_level_history();
    void set_max_history_entries(size_t max_entries);
    
    void set_level_temporary(LogLevel level, std::chrono::seconds duration, 
                            const std::string& reason = "");
    void cancel_temporary_level();
    bool has_temporary_level() const;
    std::chrono::seconds remaining_temporary_duration() const;
    
#ifndef XLOG_NO_FILTERS
    void add_filter(std::shared_ptr<LogFilter> filter);
    void clear_filters();
    void set_filter_func(std::function<bool(const LogRecord&)> func);
#endif
    
    static std::shared_ptr<Logger> create_stdout_logger(const std::string& name);
    
#ifndef XLOG_NO_ASYNC
    static std::shared_ptr<Logger> create_async(const std::string& name);
#endif
    
    std::string name;

private:
    bool should_log(const LogRecord& record) const;
    void check_temporary_level_expiry();
    void record_level_change(LogLevel old_level, LogLevel new_level, const std::string& reason);
    
    std::vector<LogSinkPtr> sinks;
#ifndef XLOG_NO_FILTERS
    std::vector<std::shared_ptr<LogFilter>> filters_;
    std::function<bool(const LogRecord&)> filter_func_;
#endif
    std::atomic<LogLevel> min_level_;
    std::vector<LogLevelChangeCallback> level_change_callbacks_;
    
    std::map<size_t, LogLevel> sink_level_overrides_;
    std::map<std::string, LogLevel> sink_level_overrides_by_name_;
    
    std::deque<LevelChangeEntry> level_history_;
    size_t max_history_entries_ = 100;
    
    TemporaryLevelChange temp_level_;
    
    std::mutex mtx;
};

using LoggerPtr = std::shared_ptr<Logger>;

struct LogLevelControlResponse {
    bool success;
    std::string message;
    LogLevel current_level;
    std::string logger_name;
    
    std::string to_json() const;
};

LogLevelControlResponse handle_level_change_request(
    std::shared_ptr<Logger> logger,
    const std::string& new_level_str,
    const std::string& reason = "",
    int duration_seconds = 0 
);


std::pair<bool, LogLevel> parse_log_level(const std::string& level_str);

}
