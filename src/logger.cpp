#include "Zyrnix/logger.hpp"
#include "Zyrnix/log_sink.hpp"
#include "Zyrnix/log_filter.hpp"
#include "Zyrnix/sinks/stdout_sink.hpp"
#include "Zyrnix/async/async_logger.hpp"
#include "Zyrnix/log_health.hpp"
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <thread>
#include <regex>

namespace Zyrnix {

void Logger::set_redact_patterns(const std::vector<std::string>& patterns) {
    std::lock_guard<std::mutex> lock(mtx_);
    redact_patterns_ = patterns;
}

void Logger::clear_redact_patterns() {
    std::lock_guard<std::mutex> lock(mtx_);
    redact_patterns_.clear();
}

void Logger::set_redact_regex_patterns(const std::vector<std::string>& patterns) {
    std::lock_guard<std::mutex> lock(mtx_);
    redact_regex_patterns_ = patterns;
}

void Logger::set_redact_pii_presets(const std::vector<std::string>& presets) {
    std::lock_guard<std::mutex> lock(mtx_);
    redact_pii_presets_ = presets;
}

void Logger::set_redact_apply_to_cloud_only(bool cloud_only) {
    std::lock_guard<std::mutex> lock(mtx_);
    redact_cloud_only_ = cloud_only;
}

Logger::Logger(std::string n) 
    : name(std::move(n)), min_level_(LogLevel::Trace) {
    temp_level_.active = false;
}

Logger::~Logger() {
    clear_sinks();
}

void Logger::add_sink(LogSinkPtr sink) {
    add_sink(std::move(sink), "");
}

void Logger::add_sink(LogSinkPtr sink, const std::string& sink_name) {
    std::unique_lock<std::shared_mutex> lock(sinks_mtx_);
    sink_entries_.push_back(std::make_shared<SinkEntry>(std::move(sink), sink_name));
}

void Logger::clear_sinks() {
    std::unique_lock<std::shared_mutex> lock(sinks_mtx_);
    
    for (auto& entry : sink_entries_) {
        entry->marked_for_removal.store(true, std::memory_order_release);
    }
    
    for (auto& entry : sink_entries_) {
        wait_for_sink_drain(entry);
    }
    
    sink_entries_.clear();
    
    std::lock_guard<std::mutex> mtx_lock(mtx_);
    sink_level_overrides_.clear();
    sink_level_overrides_by_name_.clear();
}

bool Logger::remove_sink(const std::string& sink_name, bool wait_for_completion) {
    SinkEntryPtr entry_to_remove;
    
    {
        std::unique_lock<std::shared_mutex> lock(sinks_mtx_);
        
        auto it = std::find_if(sink_entries_.begin(), sink_entries_.end(),
            [&sink_name](const SinkEntryPtr& entry) {
                return entry->name == sink_name && !entry->marked_for_removal;
            });
        
        if (it == sink_entries_.end()) {
            return false;
        }
        
        entry_to_remove = *it;
        entry_to_remove->marked_for_removal.store(true, std::memory_order_release);
    }
    
    if (wait_for_completion) {
        wait_for_sink_drain(entry_to_remove);
    }
    
    {
        std::unique_lock<std::shared_mutex> lock(sinks_mtx_);
        sink_entries_.erase(
            std::remove_if(sink_entries_.begin(), sink_entries_.end(),
                [&sink_name](const SinkEntryPtr& entry) {
                    return entry->name == sink_name && entry->marked_for_removal;
                }),
            sink_entries_.end()
        );
    }
    
    return true;
}

bool Logger::remove_sink(size_t index, bool wait_for_completion) {
    SinkEntryPtr entry_to_remove;
    
    {
        std::unique_lock<std::shared_mutex> lock(sinks_mtx_);
        
        if (index >= sink_entries_.size()) {
            return false;
        }
        
        entry_to_remove = sink_entries_[index];
        entry_to_remove->marked_for_removal.store(true, std::memory_order_release);
    }
    
    if (wait_for_completion) {
        wait_for_sink_drain(entry_to_remove);
    }
    
 
    {
        std::unique_lock<std::shared_mutex> lock(sinks_mtx_);
        cleanup_removed_sinks();
    }
    
    return true;
}

size_t Logger::sink_count() const {
    std::shared_lock<std::shared_mutex> lock(sinks_mtx_);
    return std::count_if(sink_entries_.begin(), sink_entries_.end(),
        [](const SinkEntryPtr& entry) {
            return !entry->marked_for_removal;
        });
}

void Logger::wait_for_sink_drain(SinkEntryPtr& entry) {

    constexpr int max_wait_ms = 5000;
    constexpr int sleep_interval_ms = 1;
    int waited_ms = 0;
    
    while (entry->ref_count.load(std::memory_order_acquire) > 0 && waited_ms < max_wait_ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_interval_ms));
        waited_ms += sleep_interval_ms;
    }
}

void Logger::cleanup_removed_sinks() {
    sink_entries_.erase(
        std::remove_if(sink_entries_.begin(), sink_entries_.end(),
            [](const SinkEntryPtr& entry) {
                return entry->marked_for_removal && 
                       entry->ref_count.load(std::memory_order_acquire) == 0;
            }),
        sink_entries_.end()
    );
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
        std::lock_guard<std::mutex> lock(mtx_);
        record_level_change(old_level, level, reason);
        
        for (const auto& callback : level_change_callbacks_) {
            callback(old_level, level);
        }
    }
}

void Logger::register_level_change_callback(LogLevelChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mtx_);
    level_change_callbacks_.push_back(std::move(callback));
}

void Logger::clear_level_change_callbacks() {
    std::lock_guard<std::mutex> lock(mtx_);
    level_change_callbacks_.clear();
}

void Logger::set_sink_level(size_t sink_index, LogLevel level) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::shared_lock<std::shared_mutex> sinks_lock(sinks_mtx_);
    if (sink_index < sink_entries_.size()) {
        sink_level_overrides_[sink_index] = level;
    }
}

void Logger::set_sink_level(const std::string& sink_name, LogLevel level) {
    std::lock_guard<std::mutex> lock(mtx_);
    sink_level_overrides_by_name_[sink_name] = level;
}

void Logger::clear_sink_level_overrides() {
    std::lock_guard<std::mutex> lock(mtx_);
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
    

    while (level_history_.size() > max_history_entries_) {
        level_history_.pop_front();
    }
}

std::vector<LevelChangeEntry> Logger::get_level_history(size_t max_entries) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mtx_));
    
    std::vector<LevelChangeEntry> result;
    size_t count = std::min(max_entries, level_history_.size());
    

    auto start = level_history_.end() - count;
    result.assign(start, level_history_.end());
    
    return result;
}

void Logger::clear_level_history() {
    std::lock_guard<std::mutex> lock(mtx_);
    level_history_.clear();
}

void Logger::set_max_history_entries(size_t max_entries) {
    std::lock_guard<std::mutex> lock(mtx_);
    max_history_entries_ = max_entries;

    while (level_history_.size() > max_history_entries_) {
        level_history_.pop_front();
    }
}

void Logger::set_level_temporary(LogLevel level, std::chrono::seconds duration, 
                                 const std::string& reason) {
    std::lock_guard<std::mutex> lock(mtx_);
    
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
    std::lock_guard<std::mutex> lock(mtx_);
    
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
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mtx_));
    return temp_level_.active;
}

std::chrono::seconds Logger::remaining_temporary_duration() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mtx_));
    
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
    std::lock_guard<std::mutex> lock(mtx_);
    
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
    std::lock_guard<std::mutex> lock(mtx_);
    filters_.push_back(std::move(filter));
}

void Logger::clear_filters() {
    std::lock_guard<std::mutex> lock(mtx_);
    filters_.clear();
    filter_func_ = nullptr;
}

void Logger::set_filter_func(std::function<bool(const LogRecord&)> func) {
    std::lock_guard<std::mutex> lock(mtx_);
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
    
    // Copy redaction configuration under lock
    std::vector<std::string> substr_patterns;
    std::vector<std::string> regex_patterns;
    std::vector<std::string> pii_presets;
    bool redact_cloud_only = false;

    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!should_log(record)) {
            return;
        }
        substr_patterns = redact_patterns_;
        regex_patterns = redact_regex_patterns_;
        pii_presets = redact_pii_presets_;
        redact_cloud_only = redact_cloud_only_;
    }

    // Apply redaction once and reuse for sinks that require it
    std::string redacted_message = message;
    bool has_redaction = false;

    if (!substr_patterns.empty() || !regex_patterns.empty() || !pii_presets.empty()) {
        if (!substr_patterns.empty()) {
            redacted_message = Formatter::redact(redacted_message, substr_patterns);
        }

        std::vector<std::regex> compiled;
        compiled.reserve(regex_patterns.size() + pii_presets.size());

        for (const auto& pat : regex_patterns) {
            try {
                compiled.emplace_back(pat, std::regex::ECMAScript);
            } catch (...) {
                // Ignore invalid regex patterns to avoid throwing on log path
            }
        }

        // Built-in PII presets (v1.1.3)
        for (const auto& preset : pii_presets) {
            std::string lower = preset;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
            try {
                if (lower == "email") {
                    compiled.emplace_back("[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}", std::regex::ECMAScript);
                } else if (lower == "ipv4") {
                    compiled.emplace_back("(25[0-5]|2[0-4]\\d|[01]?\\d?\\d)(\\.(25[0-5]|2[0-4]\\d|[01]?\\d?\\d)){3}", std::regex::ECMAScript);
                } else if (lower == "credit_card") {
                    compiled.emplace_back("\\b(?:\\d[ -]*?){13,16}\\b", std::regex::ECMAScript);
                } else if (lower == "ssn") {
                    compiled.emplace_back("\\b\\d{3}-\\d{2}-\\d{4}\\b", std::regex::ECMAScript);
                }
            } catch (...) {
                // Ignore preset compilation failures
            }
        }

        for (const auto& rx : compiled) {
            redacted_message = std::regex_replace(redacted_message, rx, "***");
        }

        has_redaction = (redacted_message != message);
    }

    std::shared_lock<std::shared_mutex> sinks_lock(sinks_mtx_);
    for (size_t i = 0; i < sink_entries_.size(); ++i) {
        auto& entry = sink_entries_[i];
        if (entry->marked_for_removal.load(std::memory_order_acquire)) {
            continue;
        }
        auto override_it = sink_level_overrides_.find(i);
        if (override_it != sink_level_overrides_.end()) {
            if (level < override_it->second) {
                continue;
            }
        }
        SinkGuard guard(entry);
        if (guard) {
            const bool is_cloud = guard->is_cloud_sink();
            const bool use_redacted = has_redaction && (!redact_cloud_only || is_cloud);
            const std::string& msg_to_log = use_redacted ? redacted_message : message;
            guard->log(name, level, msg_to_log);
        }
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
