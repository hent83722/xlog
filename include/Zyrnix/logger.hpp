#pragma once
#include "Zyrnix_features.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>
#include <chrono>
#include <deque>
#include <map>
#include <shared_mutex>
#include "log_sink.hpp"
#include "log_level.hpp"
#include "log_record.hpp"

namespace Zyrnix {

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

/**
 * @brief Reference-counted sink wrapper for thread-safe removal (v1.1.2)
 */
struct SinkEntry {
    LogSinkPtr sink;
    std::string name;
    std::atomic<bool> marked_for_removal{false};
    std::atomic<int> ref_count{0};
    
    SinkEntry(LogSinkPtr s, std::string n = "") 
        : sink(std::move(s)), name(std::move(n)) {}
};

using SinkEntryPtr = std::shared_ptr<SinkEntry>;

/**
 * @brief RAII guard for sink reference counting (v1.1.2)
 */
class SinkGuard {
public:
    explicit SinkGuard(SinkEntryPtr entry) : entry_(std::move(entry)) {
        if (entry_) {
            entry_->ref_count.fetch_add(1, std::memory_order_acquire);
        }
    }
    
    ~SinkGuard() {
        if (entry_) {
            entry_->ref_count.fetch_sub(1, std::memory_order_release);
        }
    }
    
    SinkGuard(const SinkGuard&) = delete;
    SinkGuard& operator=(const SinkGuard&) = delete;
    
    SinkGuard(SinkGuard&& other) noexcept : entry_(std::move(other.entry_)) {
        other.entry_ = nullptr;
    }
    
    SinkGuard& operator=(SinkGuard&& other) noexcept {
        if (this != &other) {
            if (entry_) {
                entry_->ref_count.fetch_sub(1, std::memory_order_release);
            }
            entry_ = std::move(other.entry_);
            other.entry_ = nullptr;
        }
        return *this;
    }
    
    LogSink* operator->() const { return entry_ ? entry_->sink.get() : nullptr; }
    LogSink& operator*() const { return *entry_->sink; }
    explicit operator bool() const { return entry_ && entry_->sink && !entry_->marked_for_removal; }
    
private:
    SinkEntryPtr entry_;
};

class Logger {
public:
    explicit Logger(std::string name);
    ~Logger();

    void add_sink(LogSinkPtr sink);
    void add_sink(LogSinkPtr sink, const std::string& name);
    void clear_sinks();
    bool remove_sink(const std::string& name, bool wait_for_completion = true);

    // PII/Sensitive data redaction
    void set_redact_patterns(const std::vector<std::string>& patterns);
    void clear_redact_patterns();
    // Regex-based redaction (v1.1.3)
    void set_redact_regex_patterns(const std::vector<std::string>& patterns);
    // Built-in PII presets (e.g., "email", "ipv4") (v1.1.3)
    void set_redact_pii_presets(const std::vector<std::string>& presets);
    // Control whether redaction applies only to cloud sinks (Loki, CloudWatch, Azure)
    // or to all sinks (default: all sinks).
    void set_redact_apply_to_cloud_only(bool cloud_only);
    
    /**
     * @brief Remove sink by index (v1.1.2)
     * @param index Index of the sink to remove
     * @param wait_for_completion If true, waits for active writes to complete
     * @return true if sink was found and removed
     */
    bool remove_sink(size_t index, bool wait_for_completion = true);
    
    /**
     * @brief Get number of active sinks
     */
    size_t sink_count() const;
    
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
    std::vector<std::string> redact_patterns_;
    std::vector<std::string> redact_regex_patterns_;
    std::vector<std::string> redact_pii_presets_;
    bool redact_cloud_only_ = false;
    bool should_log(const LogRecord& record) const;
    void check_temporary_level_expiry();
    void record_level_change(LogLevel old_level, LogLevel new_level, const std::string& reason);
    void cleanup_removed_sinks(); 
    void wait_for_sink_drain(SinkEntryPtr& entry); 
    

    std::vector<SinkEntryPtr> sink_entries_;
    mutable std::shared_mutex sinks_mtx_;  
    
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
    
    mutable std::mutex mtx_;  
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
