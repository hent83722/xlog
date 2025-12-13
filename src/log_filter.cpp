#include "xlog/log_filter.hpp"
#include "xlog/log_context.hpp"

namespace xlog {

LevelFilter::LevelFilter(LogLevel min_level) : min_level_(min_level) {}

bool LevelFilter::should_log(const LogRecord& record) const {
    return record.level >= min_level_;
}

FieldFilter::FieldFilter(const std::string& field_name, const std::string& expected_value)
    : field_name_(field_name), expected_value_(expected_value) {}

bool FieldFilter::should_log(const LogRecord& record) const {
    auto context = LogContext::get_all();
    auto it = context.find(field_name_);
    if (it != context.end()) {
        return it->second == expected_value_;
    }
    
    return record.get_field(field_name_) == expected_value_;
}

LambdaFilter::LambdaFilter(FilterFunc func) : filter_func_(std::move(func)) {}

bool LambdaFilter::should_log(const LogRecord& record) const {
    return filter_func_(record);
}

CompositeFilter::CompositeFilter(Mode mode) : mode_(mode) {}

void CompositeFilter::add_filter(std::shared_ptr<LogFilter> filter) {
    filters_.push_back(std::move(filter));
}

bool CompositeFilter::should_log(const LogRecord& record) const {
    if (filters_.empty()) {
        return true;
    }
    
    if (mode_ == Mode::AND) {
        for (const auto& filter : filters_) {
            if (!filter->should_log(record)) {
                return false;
            }
        }
        return true;
    } else {
        for (const auto& filter : filters_) {
            if (filter->should_log(record)) {
                return true;
            }
        }
        return false;
    }
}

RegexFilter::RegexFilter(const std::string& pattern, bool invert)
    : pattern_str_(pattern), 
      regex_(pattern), 
      invert_(invert),
      case_insensitive_(false),
      track_stats_(true) {}

RegexFilter::RegexFilter(const std::string& field_name, const std::string& pattern, bool invert)
    : pattern_str_(pattern), 
      regex_(pattern), 
      field_name_(field_name), 
      invert_(invert),
      case_insensitive_(false),
      track_stats_(true) {}

RegexFilter::RegexFilter(const std::string& pattern, const RegexFilterOptions& options)
    : pattern_str_(pattern),
      regex_(pattern, options.case_insensitive ? std::regex::icase : std::regex::ECMAScript),
      invert_(options.invert),
      case_insensitive_(options.case_insensitive),
      track_stats_(options.track_stats) {}

RegexFilter::RegexFilter(const std::string& field_name, const std::string& pattern, const RegexFilterOptions& options)
    : pattern_str_(pattern),
      regex_(pattern, options.case_insensitive ? std::regex::icase : std::regex::ECMAScript),
      field_name_(field_name),
      invert_(options.invert),
      case_insensitive_(options.case_insensitive),
      track_stats_(options.track_stats) {}

bool RegexFilter::should_log(const LogRecord& record) const {
    std::string target;
    
    if (field_name_.empty()) {
        target = record.message;
    } else {
        auto context = LogContext::get_all();
        auto it = context.find(field_name_);
        if (it != context.end()) {
            target = it->second;
        } else {
            target = record.get_field(field_name_);
        }
    }
    
    bool matches = std::regex_search(target, regex_);
    update_stats(matches);
    return invert_ ? !matches : matches;
}

void RegexFilter::update_stats(bool matched) const {
    if (track_stats_) {
        if (matched) {
            match_count_.fetch_add(1, std::memory_order_relaxed);
        } else {
            miss_count_.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

FilterStats RegexFilter::get_stats() const {
    FilterStats stats;
    stats.matches = match_count_.load(std::memory_order_relaxed);
    stats.misses = miss_count_.load(std::memory_order_relaxed);
    stats.total_checks = stats.matches + stats.misses;
    return stats;
}

void RegexFilter::reset_stats() {
    match_count_.store(0, std::memory_order_relaxed);
    miss_count_.store(0, std::memory_order_relaxed);
}

RegexFilterCache& RegexFilterCache::instance() {
    static RegexFilterCache instance;
    return instance;
}

std::string RegexFilterCache::make_cache_key(const std::string& pattern, const std::string& field,
                                             const RegexFilterOptions& options) const {
    std::string key = pattern + "|" + field + "|" +
                      (options.case_insensitive ? "i" : "") +
                      (options.invert ? "v" : "");
    return key;
}

std::shared_ptr<RegexFilter> RegexFilterCache::get_or_create(
    const std::string& pattern,
    const RegexFilterOptions& options) {
    return get_or_create("", pattern, options);
}

std::shared_ptr<RegexFilter> RegexFilterCache::get_or_create(
    const std::string& field_name,
    const std::string& pattern,
    const RegexFilterOptions& options) {
    
    std::string key = make_cache_key(pattern, field_name, options);
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        cache_hits_.fetch_add(1, std::memory_order_relaxed);
        return it->second;
    }
    
    cache_misses_.fetch_add(1, std::memory_order_relaxed);
    
    std::shared_ptr<RegexFilter> filter;
    if (field_name.empty()) {
        filter = std::make_shared<RegexFilter>(pattern, options);
    } else {
        filter = std::make_shared<RegexFilter>(field_name, pattern, options);
    }
    
    cache_[key] = filter;
    return filter;
}

void RegexFilterCache::precompile(const std::string& name, const std::string& pattern,
                                  const RegexFilterOptions& options) {
    std::lock_guard<std::mutex> lock(mutex_);
    precompiled_[name] = std::make_shared<RegexFilter>(pattern, options);
}

std::shared_ptr<RegexFilter> RegexFilterCache::get_precompiled(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = precompiled_.find(name);
    if (it != precompiled_.end()) {
        return it->second;
    }
    return nullptr;
}

void RegexFilterCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}

size_t RegexFilterCache::cache_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.size();
}

} 
