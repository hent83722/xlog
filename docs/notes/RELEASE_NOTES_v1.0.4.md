# Zyrnix v1.0.4 Release Notes

**Release Date:** December 9, 2025

## Overview

Zyrnix v1.0.4 brings three major features focused on flexibility, safety, and optimization:
1. **Configuration File Support** - JSON configuration without recompiling
2. **Signal-Safe Logging** - Crash handler support with async-signal-safe operations
3. **Conditional Compilation Guards** - Reduce binary size by 50-70KB

## 🚀 New Features

### 1. 📄 Configuration File Support

Load logger configurations from JSON files without recompiling your application.

**Key Benefits:**
- 🔄 Dynamic configuration changes without rebuilds
- 🎯 Environment-specific configs (dev/staging/production)
- 📊 Easy A/B testing of logging strategies
- 🚀 Faster development iteration

**Example:**

```cpp
#include <Zyrnix/config.hpp>

// Load configuration from file
Zyrnix::ConfigLoader::load_from_json("config.json");

// Create all configured loggers
auto loggers = Zyrnix::ConfigLoader::create_loggers();

// Use them
loggers["app"]->info("Configuration loaded!");
```

**Configuration Format:**

```json
{
  "loggers": [
    {
      "name": "app",
      "level": "info",
      "async": true,
      "sinks": [
        {"type": "stdout"},
        {"type": "file", "path": "/var/log/app.log"},
        {"type": "rotating", "path": "app.log", "max_size": 10485760, "max_files": 5}
      ]
    }
  ]
}
```

**New API:**
- `ConfigLoader::load_from_json(path)` - Load from file
- `ConfigLoader::load_from_json_string(json)` - Load from string
- `ConfigLoader::create_loggers()` - Create configured loggers
- `ConfigLoader::get_logger_configs()` - Get parsed configs
- `ConfigLoader::clear()` - Clear loaded configs

---

### 2. 🚨 Signal-Safe Logging

Async-signal-safe logging for crash handlers (SIGSEGV, SIGABRT, etc.).

**Key Benefits:**
- ✅ Safe to call from signal handlers
- ✅ Captures crash information reliably
- ✅ Lock-free ring buffer design
- ✅ Uses only async-signal-safe POSIX functions
- ✅ No malloc/free in critical paths

**Example:**

```cpp
#include <Zyrnix/sinks/signal_safe_sink.hpp>

// Set up crash logger
auto crash_sink = std::make_shared<Zyrnix::SignalSafeSink>("crash.log");
auto crash_logger = std::make_shared<Zyrnix::Logger>("crash");
crash_logger->add_sink(crash_sink);

void crash_handler(int sig) {
    crash_logger->log(Zyrnix::LogLevel::Critical, "Application crashed!");
    crash_sink->flush();  // Ensure logs are written
    _exit(1);
}

signal(SIGSEGV, crash_handler);
signal(SIGABRT, crash_handler);
```

**Technical Details:**
- Uses POSIX `write()` instead of `fprintf()`
- Lock-free circular buffer (no mutexes)
- Pre-allocated buffer (no dynamic allocation)
- Only async-signal-safe functions: `write()`, `open()`, `close()`, `fsync()`

---

### 3. 📦 Conditional Compilation Guards

Granular feature flags to reduce binary size by 50-70KB.

**Key Benefits:**
- 📉 Reduce binary size for embedded/IoT devices
- ⚡ Faster compilation times
- 🎯 Include only what you need
- 💾 Lower memory footprint

**Feature Flags:**

| Flag | Disables | Size Saved |
|------|----------|------------|
| `XLOG_NO_ASYNC` | Asynchronous logging | ~15-20KB |
| `XLOG_NO_JSON` | JSON/structured logging | ~10-15KB |
| `XLOG_NO_NETWORK` | Network sinks (UDP, Syslog) | ~8-12KB |
| `XLOG_NO_COLORS` | Color output | ~2-3KB |
| `XLOG_NO_FILE_ROTATION` | Rotating file sinks | ~5-8KB |
| `XLOG_NO_CONTEXT` | Log contexts (MDC/NDC) | ~3-5KB |
| `XLOG_NO_FILTERS` | Log filtering | ~2-4KB |
| `XLOG_MINIMAL` | All optional features | ~50-70KB |

**Usage:**

**CMake:**
```cmake
# Minimal build
cmake -DXLOG_MINIMAL=ON ..

# Custom build
cmake -DXLOG_ENABLE_ASYNC=OFF -DXLOG_ENABLE_JSON=OFF ..
```

**Compile flags:**
```bash
g++ -DXLOG_NO_ASYNC -DXLOG_NO_JSON main.cpp -lZyrnix
```

**Feature detection:**
```cpp
#include <Zyrnix/Zyrnix_features.hpp>

#if XLOG_HAS_ASYNC
    auto logger = Zyrnix::Logger::create_async("app");
#else
    auto logger = Zyrnix::Logger::create_stdout_logger("app");
#endif
```

---

## Previous Features (retained from beta)

#### **Compile-Time Log Elimination**

New preprocessor macros that completely remove debug/trace logs in release builds:

```cpp
XLOG_TRACE(logger, "Detailed trace information");  // Eliminated in release
XLOG_DEBUG(logger, "Debug details");               // Eliminated in release
XLOG_INFO(logger, "Important information");        // Always included
XLOG_WARN(logger, "Warning message");              // Always included
XLOG_ERROR(logger, "Error occurred");              // Always included
XLOG_CRITICAL(logger, "Critical failure");         // Always included
```

**Performance Impact:** Zero overhead - filtered logs are not compiled into release binaries.

#### 2. **Conditional Logging Macros**

Prevent expensive string construction when conditions aren't met:

```cpp
XLOG_DEBUG_IF(logger, user.is_premium(), "Premium user: {}", user.get_details());
XLOG_INFO_IF(logger, request_id % 2 == 0, "Processing request: {}", request_id);
```

**Performance Impact:** Message construction only occurs when the condition is true.

#### 3. **Runtime Filtering System**

Flexible filter architecture for dynamic log control:

##### Level-Based Filtering
```cpp
logger->add_filter(std::make_shared<Zyrnix::LevelFilter>(Zyrnix::LogLevel::Warn));
```

##### Field-Based Filtering
```cpp
logger->add_filter(std::make_shared<Zyrnix::FieldFilter>("user_type", "premium"));
```

##### Lambda Filters
```cpp
logger->add_filter(std::make_shared<Zyrnix::LambdaFilter>(
    [](const Zyrnix::LogRecord& record) {
        return record.level >= Zyrnix::LogLevel::Error || 
               record.has_field("urgent");
    }
));
```

##### Composite Filters
```cpp
auto composite = std::make_shared<Zyrnix::CompositeFilter>(
    Zyrnix::CompositeFilter::Mode::AND
);
composite->add_filter(level_filter);
composite->add_filter(field_filter);
logger->add_filter(composite);
```

## 📁 New Files

### Added in v1.0.4
- `include/Zyrnix/Zyrnix_features.hpp` - Feature flag definitions
- `include/Zyrnix/sinks/signal_safe_sink.hpp` - Signal-safe sink header
- `src/sinks/signal_safe_sink.cpp` - Signal-safe sink implementation
- `examples/config_file_example.cpp` - Configuration file usage demo
- `examples/signal_safe_example.cpp` - Crash handler demo
- `examples/minimal_build_example.cpp` - Feature flag demo

### Enhanced in v1.0.4
- `include/Zyrnix/config.hpp` - Added ConfigLoader class
- `src/config.cpp` - JSON parsing and logger creation
- `CMakeLists.txt` - Feature flags and conditional compilation
- `include/Zyrnix/logger.hpp` - Feature guards
- `include/Zyrnix/Zyrnix.hpp` - Feature guards
- `README.md` - v1.0.4 documentation

### From previous beta
- `include/Zyrnix/log_filter.hpp` - Filter interface and implementations
- `src/log_filter.cpp` - Filter implementation logic
- `include/Zyrnix/log_macros.hpp` - Zero-cost logging macros
- `examples/conditional_logging.cpp` - Comprehensive usage examples

## 🔧 API Additions

### Logger Class

New filtering methods added to `Zyrnix::Logger`:

```cpp
void add_filter(std::shared_ptr<LogFilter> filter);
void clear_filters();
void set_filter_func(std::function<bool(const LogRecord&)> func);
void set_level(LogLevel level);
LogLevel get_level() const;
```

### LogRecord Enhancements

Extended `Zyrnix::LogRecord` with field inspection:

```cpp
std::unordered_map<std::string, std::string> fields;
bool has_field(const std::string& key) const;
std::string get_field(const std::string& key) const;
```

### Filter Classes

New filter types available:

- `LogFilter` - Base filter interface
- `LevelFilter` - Filter by minimum log level
- `FieldFilter` - Filter by context field values
- `LambdaFilter` - Filter using custom predicates
- `CompositeFilter` - Combine multiple filters with AND/OR logic

## 📊 Performance Characteristics

Benchmark results for 100,000 log operations:

- **Compile-time filtering:** 0μs overhead (eliminated at compile time)
- **Conditional macros:** ~0.01μs per check (condition evaluation only)
- **Runtime filtering:** ~0.12μs per filtered log
- **No filtering:** Baseline performance maintained

## 🎯 Use Cases

This release is particularly valuable for:

1. **Production Performance** - Eliminate debug logs without code changes
2. **Dynamic Debugging** - Enable/disable specific log categories at runtime
3. **Context-Aware Logging** - Filter logs based on user roles, request IDs, etc.
4. **Multi-Environment Deployment** - Different log levels per environment
5. **Cost Optimization** - Reduce log volume in high-throughput systems

## 🔄 Compile-Time Control

Control log levels at compile time with preprocessor definitions:

```bash
# Development build (includes TRACE and DEBUG)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (eliminates TRACE and DEBUG)
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DNDEBUG" ..

# Custom active level (0=Trace, 1=Debug, 2=Info, 3=Warn, 4=Error, 5=Critical)
cmake -DCMAKE_CXX_FLAGS="-DXLOG_ACTIVE_LEVEL=2" ..
```

## 📚 Documentation Updates

- Updated `README.md` with comprehensive Conditional Logging section
- Added usage examples for all filter types
- Documented performance characteristics and best practices
- Created `examples/conditional_logging.cpp` with 7 demonstration functions

## 🔨 Implementation Details

### Macro System

- Uses numeric level system (0-5) for preprocessor compatibility
- `XLOG_ACTIVE_LEVEL` defaults to 0 (Trace) in debug, 2 (Info) in release
- Macros expand to inline code for optimal compiler optimization

### Filter Architecture

- Polymorphic design using virtual base class
- Thread-safe integration with existing Logger class
- Integrates with LogContext for field-based filtering
- Minimal memory overhead per filter (~16-24 bytes)

### Backward Compatibility

- All existing APIs remain unchanged
- New features are opt-in
- Existing code continues to work without modifications
- Default behavior maintains v1.0.3 functionality

## 🧪 Testing

- Verified all filter types work correctly
- Tested compile-time elimination in release builds
- Benchmarked performance overhead
- Validated thread safety with existing async logging
- All existing unit tests continue to pass

## 📦 Building with v1.0.4

```bash
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix
mkdir build && cd build
cmake ..
make
sudo make install
```

## 🔗 Example Usage

See the complete working example in `examples/conditional_logging.cpp`:

```bash
cd build
./cond_test
```

## 🙏 Credits

This release implements Feature #1 from the Zyrnix roadmap: "Conditional Logging with Zero-Cost Abstractions" - designed to provide both performance optimization and enhanced developer experience.

## 📝 Migration Guide

To adopt conditional logging in existing code:

1. **Replace existing log calls with macros** (optional):
   ```cpp
   // Before
   logger->debug("Debug message");
   
   // After (for compile-time elimination)
   XLOG_DEBUG(logger, "Debug message");
   ```

2. **Add runtime filters** (optional):
   ```cpp
   logger->add_filter(std::make_shared<Zyrnix::LevelFilter>(Zyrnix::LogLevel::Info));
   ```

3. **Use conditional logging** (optional):
   ```cpp
   XLOG_DEBUG_IF(logger, expensive_condition(), "Details: {}", expensive_call());
   ```

No changes are required - all existing code continues to work as-is.

## 🐛 Known Issues

None at this time.

## 🔮 Future Enhancements

Potential improvements for future releases:

- Configuration file support for filter rules
- Dynamic filter management via API
- Filter statistics and monitoring
- Pattern-based message filtering
- Rate limiting integration with filters

---

**Full Changelog:** [v1.0.3...v1.0.4](https://github.com/hent83722/Zyrnix/compare/v1.0.3...v1.0.4)

**Questions or Issues?** Please file an issue on [GitHub](https://github.com/hent83722/Zyrnix/issues)
