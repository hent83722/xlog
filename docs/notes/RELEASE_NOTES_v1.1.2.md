# XLog v1.1.2 Release Notes

**Release Date:** December 13, 2025  
**Release Type:** Bug Fixes & Hardening  
**Stability:** Stable

---

## 🎯 Overview

XLog v1.1.2 is a stability-focused release that addresses critical bug fixes and hardening improvements. This release ensures more reliable shutdown behavior, safer concurrent sink management, proper Windows Unicode path support, and improved signal handler safety.

---

## 🔧 Bug Fixes & Hardening

### 1. Flush Guarantees on Shutdown

**Problem:** Async logging queues could lose messages during application shutdown if the queue wasn't fully drained before destruction.

**Solution:** 
- `AsyncQueue` now implements configurable shutdown timeout with guaranteed drain behavior
- Destructor waits for queue to fully drain (with timeout protection)
- New `shutdown()` method with explicit drain control
- Tracks dropped messages if timeout occurs

**API Additions:**
```cpp
// AsyncQueue now supports:
AsyncQueue(size_t shutdown_timeout_ms = 5000);  // Configurable timeout
bool shutdown(bool wait_for_drain = true);       // Graceful shutdown
bool is_shutting_down() const;                   // Check shutdown state
void set_shutdown_timeout(size_t timeout_ms);    // Adjust timeout
size_t dropped_on_shutdown() const;              // Get drop count
size_t size() const;                             // Get queue size
```

**Impact:** No more lost log messages on application exit. Default 5-second timeout prevents hangs.

---

### 2. Thread-Safe Sink Removal

**Problem:** Removing sinks while logging was occurring could cause race conditions or block writers indefinitely.

**Solution:**
- Implemented reference counting for active sink usage via `SinkEntry` wrapper
- `SinkGuard` RAII class manages reference counts automatically
- Sinks are marked for removal and wait for active references to complete
- Uses `std::shared_mutex` (reader-writer lock) for better concurrency

**API Additions:**
```cpp
// Logger now supports:
void add_sink(LogSinkPtr sink, const std::string& name);  // Named sinks
bool remove_sink(const std::string& name, bool wait = true);  // Remove by name
bool remove_sink(size_t index, bool wait = true);             // Remove by index
size_t sink_count() const;                                     // Get active count
```

**Impact:** Hot-removal of sinks without blocking writers. Safe concurrent logging during sink management.

---

### 3. Windows Unicode Path Support

**Problem:** File paths containing non-ASCII characters (e.g., `C:\Users\日本語\logs\app.log`) would fail on Windows.

**Solution:**
- Added `xlog::path` namespace with Unicode-aware file operations
- Proper UTF-8 → UTF-16 conversion for Windows APIs
- Cross-platform API that's transparent on POSIX systems

**API Additions:**
```cpp
namespace xlog::path {
    // Windows: UTF-8 to UTF-16, POSIX: passthrough
    std::wstring to_native(const std::string& utf8_path);  // Windows only
    std::string from_native(const std::wstring& native);   // Windows only
    
    // Cross-platform Unicode-safe operations
    FILE* fopen_utf8(const std::string& path, const char* mode);
    bool file_exists(const std::string& path);
    bool create_directory(const std::string& path);
    bool rename_file(const std::string& old_path, const std::string& new_path);
    bool remove_file(const std::string& path);
}
```

**Updated Sinks:**
- `FileSink` - Now opens files with Unicode support
- `RotatingFileSink` - Unicode paths for rotation operations

**Impact:** Full international character support in file paths on Windows.

---

### 4. Signal Handler Reentrancy Fixes

**Problem:** `SignalSafeSink` could exhibit undefined behavior when signals arrived in rapid succession or nested.

**Solution:**
- Audited all code paths for async-signal-safety compliance
- Implemented lock-free write position reservation with `compare_exchange`
- Added `flush_in_progress_` flag to prevent concurrent flush operations
- Handle `EINTR` properly in `write()` syscall
- Added `O_CLOEXEC` flag to prevent fd leaks to child processes
- Track dropped messages when buffer is full

**API Additions:**
```cpp
class SignalSafeSink {
    // Existing methods enhanced for reentrancy
    
    // v1.1.2 additions:
    size_t dropped_count() const;           // Messages dropped due to buffer full
    bool in_signal_context() const;         // Check if in signal handler
    void enter_signal_handler();            // Mark signal handler entry
    void exit_signal_handler();             // Mark signal handler exit
};
```

**Technical Improvements:**
- Lock-free buffer write with atomic compare-exchange
- Proper handling of `EINTR` (interrupted system calls)
- Static level-to-string function (no memory allocation)
- Async-signal-safe integer-to-string conversion

**Impact:** Reliable crash logging even under rapid successive signals.

---

## 📊 Performance Impact

| Operation | v1.1.1 | v1.1.2 | Change |
|-----------|--------|--------|--------|
| Sink removal during logging | May block | Non-blocking | ✅ Improved |
| Logger destruction | May lose msgs | Guaranteed flush | ✅ Improved |
| Unicode path operations (Win) | May fail | Works correctly | ✅ Fixed |
| Signal handler logging | May corrupt | Fully reentrant | ✅ Fixed |

---

## 🔄 Migration Guide

### From v1.1.1 to v1.1.2

**No breaking changes.** This is a drop-in replacement.

**Recommended updates:**

1. **Use named sinks for easier management:**
   ```cpp
   // Before (still works)
   logger->add_sink(std::make_shared<FileSink>("app.log"));
   
   // After (recommended)
   logger->add_sink(std::make_shared<FileSink>("app.log"), "file");
   logger->remove_sink("file");  // Easy removal
   ```

2. **Configure shutdown timeout if needed:**
   ```cpp
   // For long-running async operations
   AsyncQueue queue(10000);  // 10 second timeout
   ```

3. **Use path utilities for cross-platform code:**
   ```cpp
   // Cross-platform Unicode-safe
   FILE* f = xlog::path::fopen_utf8(user_provided_path, "w");
   ```

---

## 🧪 Testing

All features have been validated with:
- ✅ Unit tests for each new API
- ✅ Concurrent stress tests (4-8 threads)
- ✅ Rapid successive signal simulation
- ✅ Sink removal during active logging
- ✅ Shutdown drain verification
- ✅ Cross-platform path operations

---

## 📁 Files Changed

### Headers Modified
- `include/xlog/logger.hpp` - SinkEntry, SinkGuard, new Logger methods
- `include/xlog/async/async_queue.hpp` - Shutdown API, size tracking
- `include/xlog/sinks/signal_safe_sink.hpp` - Reentrancy improvements
- `include/xlog/util.hpp` - Path utilities namespace

### Sources Modified
- `src/logger.cpp` - Reference counting, shared_mutex, sink management
- `src/async/async_queue.cpp` - Full implementation of shutdown behavior
- `src/sinks/signal_safe_sink.cpp` - Lock-free writes, EINTR handling
- `src/sinks/file_sink.cpp` - Unicode path support
- `src/sinks/rotating_file_sink.cpp` - Unicode path support
- `src/util.cpp` - Path utility implementations

---

## 🔮 What's Next

v1.2.0 will focus on new features:
- Grafana Loki sink
- PII/Sensitive data redaction
- Hot-reload configuration
- Additional cloud integrations

---

## 🙏 Acknowledgments

Thanks to all users who reported issues with:
- Log loss during shutdown
- Windows international path support
- Crash handler reliability

---

## 📝 Full Changelog

See [CHANGELOG.md](../../CHANGELOG.md) for complete version history.

---

**Download:** [GitHub Releases](https://github.com/hent83722/xlog/releases/tag/v1.1.2)

**Documentation:** [docs/](../../docs/)

**Report Issues:** [GitHub Issues](https://github.com/hent83722/xlog/issues)
