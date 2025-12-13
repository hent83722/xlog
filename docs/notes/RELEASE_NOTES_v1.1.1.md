# XLog v1.1.1 Release Notes

**Release Date**: December 13, 2025  
**Version**: 1.1.1  
**Previous Version**: 1.1.0  
**Status**: Stable Release

---

## 🎉 Overview

XLog v1.1.1 is a feature-rich release focused on **production operations**, **observability**, and **developer experience**. This release consolidates improvements from beta.1 and beta.2 into a stable, production-ready package.

### Highlights

- 🎯 **Regex Filter Caching & Optimization** - High-performance pattern filtering with statistics
- 🏥 **Enhanced Health Monitoring** - Auto-registration, aggregate checks, error tracking
- 🔄 **Advanced Dynamic Log Levels** - Audit trails, temporary changes, per-sink overrides, REST API
- 🗜️ **Compression Auto-Tune** - Adaptive compression level optimization

---

## ✨ New Features

### 1. 🎯 Regex-Based Log Filtering

Pattern-based filtering using regular expressions for flexible log control.

**Capabilities:**
- Filter by message content or structured fields
- Inverted matching (exclude patterns)
- Composite filters (AND/OR logic)
- Thread-safe, lock-free operation

```cpp
#include <xlog/log_filter.hpp>

// Basic pattern filtering
auto error_filter = std::make_shared<xlog::RegexFilter>("(ERROR|CRITICAL)");
logger->add_filter(error_filter);

// Inverted: exclude sensitive data
auto no_secrets = std::make_shared<xlog::RegexFilter>(
    "(password|token|secret)", true);  // invert = true
logger->add_filter(no_secrets);

// Composite filters
auto composite = std::make_shared<xlog::CompositeFilter>(
    xlog::CompositeFilter::Mode::AND);
composite->add_filter(error_filter);
composite->add_filter(no_secrets);
```

---

### 2. 🚀 Regex Filter Caching & Optimization

Enhanced regex filtering with performance optimizations and statistics tracking.

**Capabilities:**
- Pre-compiled static filters for pattern reuse
- Filter statistics (matches/misses/rate)
- Case-insensitive matching option
- Automatic caching to avoid recompilation

```cpp
// Case-insensitive matching with statistics
xlog::RegexFilterOptions options;
options.case_insensitive = true;
options.track_stats = true;

auto filter = std::make_shared<xlog::RegexFilter>("error|warning", options);
logger->add_filter(filter);

// Check statistics
auto stats = filter->get_stats();
std::cout << "Match rate: " << (stats.match_rate() * 100) << "%\n";

// Pre-compile commonly used patterns
auto& cache = xlog::RegexFilterCache::instance();
cache.precompile("sensitive_data", "(password|token|secret)",
                 {.case_insensitive = true, .invert = true, .track_stats = true});

// Reuse anywhere
auto cached = cache.get_precompiled("sensitive_data");
```

**Performance:**
- Cache lookup: O(1) hash table
- Statistics overhead: <50ns per check
- Eliminates regex compilation cost (~10-100μs)

---

### 3. 🔄 Dynamic Log Level Changes

Runtime log level adjustments without restart, using lock-free atomic operations.

**Capabilities:**
- Thread-safe level changes
- Level change callbacks for monitoring
- No performance impact on logging path

```cpp
auto logger = xlog::Logger::create_stdout_logger("app");

// Thread-safe runtime level change
logger->set_level_dynamic(xlog::LogLevel::Debug);

// Register callback for monitoring
logger->register_level_change_callback(
    [](LogLevel old_level, LogLevel new_level) {
        metrics.record("log_level_change", new_level);
    });
```

**Performance:**
- Read path: <10ns (lock-free atomic load)
- Write path: ~100ns (atomic exchange + callbacks)

---

### 4. 📜 Level Change History & Audit Trail

Track all log level changes with timestamps and reasons for compliance and debugging.

**Capabilities:**
- Level change audit trail with timestamps
- Reason tracking for each change
- Configurable history size

```cpp
// Change with reason (audit trail)
logger->set_level_dynamic(LogLevel::Debug, "Debugging issue #12345");

// View history
auto history = logger->get_level_history();
for (const auto& entry : history) {
    std::cout << entry.timestamp << " - " << entry.reason
              << " (" << entry.old_level << " → " << entry.new_level << ")\n";
}

// Configure history size
logger->set_max_history_entries(50);
logger->clear_level_history();
```

---

### 5. ⏱️ Temporary Level Changes

Enable verbose logging for a limited time with automatic revert.

**Capabilities:**
- Auto-revert after specified duration
- Manual cancellation
- Original level preserved

```cpp
// Enable trace for 5 minutes
logger->set_level_temporary(LogLevel::Trace,
                           std::chrono::seconds(300),
                           "Debugging production issue");

// Check status
if (logger->has_temporary_level()) {
    auto remaining = logger->remaining_temporary_duration();
    std::cout << "Reverting in " << remaining.count() << "s\n";
}

// Cancel early
logger->cancel_temporary_level();
```

---

### 6. 🎚️ Per-Sink Level Overrides

Different log levels for different sinks - debug to file, info to console.

```cpp
auto logger = std::make_shared<xlog::Logger>("app");
logger->add_sink(std::make_shared<xlog::StdoutSink>());  // sink 0
logger->add_sink(std::make_shared<xlog::FileSink>("debug.log"));  // sink 1

// Console: Info and above
logger->set_sink_level(0, LogLevel::Info);

// File: Debug and above
logger->set_sink_level(1, LogLevel::Debug);

// Clear overrides
logger->clear_sink_level_overrides();
```

---

### 7. 🌐 REST API Helpers

Built-in support for web-based log level control.

```cpp
// Handle REST API request
auto response = xlog::handle_level_change_request(
    logger,
    "debug",           // New level string
    "Admin request",   // Reason
    300                // Duration (0 = permanent)
);

// Returns JSON response
std::cout << response.to_json();
// {"success": true, "message": "...", "logger_name": "app", "current_level": "debug"}

// Parse level strings
auto [ok, level] = xlog::parse_log_level("debug");  // case-insensitive
```

---

### 8. 🏥 Health Check API

Built-in health monitoring for SRE practices and observability.

**Capabilities:**
- Real-time status assessment
- Configurable thresholds
- JSON output for monitoring systems

```cpp
#include <xlog/log_health.hpp>

// Register logger for monitoring
auto& registry = xlog::HealthRegistry::instance();
registry.register_logger("app", logger);

// Check health
auto result = registry.check_logger("app");
std::cout << result.to_json();

// Kubernetes-style endpoint
std::string handle_health(const Request& req) {
    return xlog::handle_health_check_request(req.query("logger"));
}
```

---

### 9. 🤖 Auto-Registration & Aggregate Health

Automatic logger registration and system-wide health overview.

**Capabilities:**
- Auto-registration on logger creation
- Aggregate health check for all loggers
- Per-logger configuration
- Health state change callbacks

```cpp
// Enable auto-registration
xlog::HealthRegistry::enable_auto_registration(true);

auto api = Logger::create_stdout_logger("api");      // Auto-registered
auto db = Logger::create_stdout_logger("database");  // Auto-registered

// Stricter thresholds for critical loggers
xlog::HealthCheckConfig strict;
strict.max_drop_rate_healthy = 0.001;  // 0.1%
strict.max_latency_us_healthy = 5000;   // 5ms
registry.set_logger_config("api", strict);

// State change callbacks
registry.register_state_change_callback(
    [](const std::string& name, HealthStatus old_s,
       HealthStatus new_s, const HealthCheckResult& result) {
        if (new_s == HealthStatus::Unhealthy) {
            send_alert(name, result.message);
        }
    });

// Aggregate check (perfect for K8s probes)
auto aggregate = xlog::handle_aggregate_health_check();
std::cout << aggregate.to_json();
```

---

### 10. 🔍 Error Tracking

Track and report last error for debugging.

```cpp
// Record error
registry.record_error("database", "Connection timeout to primary");

// Error appears in health check
auto result = registry.check_logger("database");
std::cout << result.last_error_message;  // "Connection timeout to primary"
std::cout << result.last_error_time;     // When it occurred
```

---

### 11. 🗜️ Compression Auto-Tune

Adaptive compression level optimization based on workload characteristics.

```cpp
#include <xlog/sinks/compressed_file_sink.hpp>

xlog::CompressionConfig config;
config.algorithm = xlog::CompressionAlgorithm::Zstd;
config.auto_tune = true;  // Enable adaptive optimization

auto sink = std::make_shared<xlog::CompressedFileSink>("app.log", config);
logger->add_sink(sink);
```

---

## 📦 API Reference

### New Structures

```cpp
// Filter statistics
struct FilterStats {
    uint64_t matches;
    uint64_t misses;
    uint64_t total_checks;
    double match_rate() const;
};

// Filter options
struct RegexFilterOptions {
    bool case_insensitive = false;
    bool invert = false;
    bool track_stats = true;
};

// Level change entry (audit trail)
struct LevelChangeEntry {
    LogLevel old_level;
    LogLevel new_level;
    std::chrono::system_clock::time_point timestamp;
    std::string reason;
};

// Aggregate health result
struct AggregateHealthResult {
    HealthStatus overall_status;
    size_t total_loggers;
    size_t healthy_count;
    size_t degraded_count;
    size_t unhealthy_count;
    std::string worst_logger_name;
    std::map<std::string, HealthCheckResult> individual_results;
    std::string to_json() const;
    std::string to_string() const;
};

// REST API response
struct LogLevelControlResponse {
    bool success;
    std::string message;
    LogLevel current_level;
    std::string logger_name;
    std::string to_json() const;
};
```

### New Classes

```cpp
// Regex filter cache singleton
class RegexFilterCache {
    static RegexFilterCache& instance();
    std::shared_ptr<RegexFilter> get_or_create(const std::string& pattern, ...);
    void precompile(const std::string& name, const std::string& pattern, ...);
    std::shared_ptr<RegexFilter> get_precompiled(const std::string& name);
    void clear();
    size_t cache_size() const;
    size_t cache_hits() const;
    size_t cache_misses() const;
};
```

### New Logger Methods

```cpp
class Logger {
    // Dynamic levels with audit
    void set_level_dynamic(LogLevel level, const std::string& reason);
    std::vector<LevelChangeEntry> get_level_history(size_t max = 100) const;
    void clear_level_history();
    void set_max_history_entries(size_t max);
    
    // Temporary levels
    void set_level_temporary(LogLevel level, std::chrono::seconds duration,
                            const std::string& reason = "");
    void cancel_temporary_level();
    bool has_temporary_level() const;
    std::chrono::seconds remaining_temporary_duration() const;
    
    // Per-sink overrides
    void set_sink_level(size_t index, LogLevel level);
    void set_sink_level(const std::string& name, LogLevel level);
    void clear_sink_level_overrides();
};
```

### New HealthRegistry Methods

```cpp
class HealthRegistry {
    void register_logger(const std::string& name, std::shared_ptr<Logger> logger,
                        const HealthCheckConfig& config);
    void set_logger_config(const std::string& name, const HealthCheckConfig& config);
    void register_state_change_callback(HealthStateChangeCallback cb);
    void clear_state_change_callbacks();
    void record_error(const std::string& name, const std::string& message);
    AggregateHealthResult check_all_aggregate() const;
    
    static void enable_auto_registration(bool enable);
    static bool is_auto_registration_enabled();
};
```

### Helper Functions

```cpp
// REST API helpers
LogLevelControlResponse handle_level_change_request(
    std::shared_ptr<Logger> logger,
    const std::string& level_str,
    const std::string& reason = "",
    int duration_seconds = 0);

std::pair<bool, LogLevel> parse_log_level(const std::string& level_str);

AggregateHealthResult handle_aggregate_health_check();
```

---

## 🧪 Testing

All features validated with comprehensive QA test suite:

- **45 tests** covering all v1.1.1 features
- Thread safety tests with concurrent access
- Integration tests combining multiple features
- Performance regression tests

Run tests:
```bash
cd qa-tests/build
cmake .. && make
./run_all_qa_tests
```

---

## 📊 Performance

| Feature | Overhead |
|---------|----------|
| Regex filter check | 1-5 μs |
| Filter stats tracking | <50 ns |
| Cache lookup | O(1) |
| Dynamic level read | <10 ns |
| Dynamic level write | ~100 ns |
| Health check | <1 ms |

---

## 🔄 Migration Guide

### From v1.1.0

v1.1.1 is fully backward compatible. Existing code works without changes.

**Optional migrations:**
- Replace `set_level()` with `set_level_dynamic()` for thread-safe changes
- Add `HealthRegistry::enable_auto_registration(true)` for automatic monitoring
- Use `RegexFilterCache` for frequently-used patterns

---

## 🐛 Bug Fixes

- Fixed thread safety issue in level change callbacks
- Fixed memory leak in regex filter destruction
- Fixed health check JSON escaping for special characters

---

## 📚 Documentation

- [Architecture Guide](../architecture.md)
- [Configuration Reference](../config.md)
- [Performance Tuning](../performance.md)
- [Examples](../examples.md)

---

## 🙏 Contributors

Thanks to all contributors who made this release possible!

---

**Full Changelog**: v1.1.0...v1.1.1
