#pragma once
#include <string>
#include <functional>
#include <memory>
#include <regex>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include "log_level.hpp"
#include "log_record.hpp"

namespace Zyrnix {

class LogFilter {
public:
    virtual ~LogFilter() = default;
    virtual bool should_log(const LogRecord& record) const = 0;
};

class LevelFilter : public LogFilter {
public:
    explicit LevelFilter(LogLevel min_level);
    bool should_log(const LogRecord& record) const override;

private:
    LogLevel min_level_;
};

class FieldFilter : public LogFilter {
public:
    FieldFilter(const std::string& field_name, const std::string& expected_value);
    bool should_log(const LogRecord& record) const override;

private:
    std::string field_name_;
    std::string expected_value_;
};

class LambdaFilter : public LogFilter {
public:
    using FilterFunc = std::function<bool(const LogRecord&)>;
    explicit LambdaFilter(FilterFunc func);
    bool should_log(const LogRecord& record) const override;

private:
    FilterFunc filter_func_;
};

class CompositeFilter : public LogFilter {
public:
    enum class Mode { AND, OR };
    
    CompositeFilter(Mode mode);
    void add_filter(std::shared_ptr<LogFilter> filter);
    bool should_log(const LogRecord& record) const override;

private:
    Mode mode_;
    std::vector<std::shared_ptr<LogFilter>> filters_;
};


struct FilterStats {
    uint64_t matches{0};
    uint64_t misses{0};
    uint64_t total_checks{0};
    
    double match_rate() const {
        return total_checks > 0 ? static_cast<double>(matches) / total_checks : 0.0;
    }
};

struct RegexFilterOptions {
    bool case_insensitive = false;
    bool invert = false;
    bool track_stats = true;
};

class RegexFilter : public LogFilter {
public:
    explicit RegexFilter(const std::string& pattern, bool invert = false);
    
    RegexFilter(const std::string& field_name, const std::string& pattern, bool invert = false);
    
    RegexFilter(const std::string& pattern, const RegexFilterOptions& options);
    RegexFilter(const std::string& field_name, const std::string& pattern, const RegexFilterOptions& options);
    
    bool should_log(const LogRecord& record) const override;
    
    std::string pattern() const { return pattern_str_; }
    bool is_case_insensitive() const { return case_insensitive_; }
    bool is_inverted() const { return invert_; }
    
    FilterStats get_stats() const;
    void reset_stats();
    
private:
    std::string pattern_str_;
    std::regex regex_;
    std::string field_name_;  
    bool invert_;
    bool case_insensitive_;
    bool track_stats_;
    
    mutable std::atomic<uint64_t> match_count_{0};
    mutable std::atomic<uint64_t> miss_count_{0};
    
    void update_stats(bool matched) const;
};

class RegexFilterCache {
public:
    static RegexFilterCache& instance();
    
    std::shared_ptr<RegexFilter> get_or_create(
        const std::string& pattern,
        const RegexFilterOptions& options = RegexFilterOptions{});
    
    std::shared_ptr<RegexFilter> get_or_create(
        const std::string& field_name,
        const std::string& pattern,
        const RegexFilterOptions& options = RegexFilterOptions{});
    
    void precompile(const std::string& name, const std::string& pattern,
                   const RegexFilterOptions& options = RegexFilterOptions{});
    
    std::shared_ptr<RegexFilter> get_precompiled(const std::string& name) const;
    
    void clear();
    
    size_t cache_size() const;
    size_t cache_hits() const { return cache_hits_.load(); }
    size_t cache_misses() const { return cache_misses_.load(); }
    
private:
    RegexFilterCache() = default;
    
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<RegexFilter>> cache_;
    std::unordered_map<std::string, std::shared_ptr<RegexFilter>> precompiled_;
    mutable std::atomic<size_t> cache_hits_{0};
    mutable std::atomic<size_t> cache_misses_{0};
    
    std::string make_cache_key(const std::string& pattern, const std::string& field,
                               const RegexFilterOptions& options) const;
};

} 