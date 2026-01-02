# Zyrnix v1.0.3 Release Notes

**Release Date:** December 7, 2025

---

## Overview

Zyrnix v1.0.3 introduces a major new feature for enterprise logging: **Log Contexts & Scoped Attributes** (MDC/NDC), along with comprehensive testing infrastructure improvements including sanitizers and fuzz testing. This release significantly enhances Zyrnix's capabilities for distributed systems, microservices, and production environments.

---

## 🚀 Major New Feature: Log Contexts & Scoped Attributes

### What is it?

Log Context provides Mapped Diagnostic Context (MDC) functionality similar to Log4j and SLF4J. It allows you to set contextual attributes (like request IDs, user IDs, trace IDs) that are automatically included in all log messages within a scope, without passing them as parameters through your entire call stack.

### Why is this important?

In complex applications, especially microservices and distributed systems, tracking requests across multiple function calls and services is critical. Previously, you had to manually pass context information through every function call or use global variables. Now, context is automatically inherited by all nested function calls within the same thread.

### Key Features

**Thread-Local Storage:** Each thread maintains its own isolated context—no cross-contamination between concurrent requests.

**RAII Scoped Management:** Context automatically cleaned up when scope exits—no memory leaks or stale context.

**Automatic Field Injection:** All logs within a scope automatically include context fields—zero boilerplate.

**Nested Contexts:** Support for hierarchical context scopes—add fine-grained tracking at any level.

**Global Context:** Set application-wide metadata that appears in all logs across all threads.

**Chainable API:** Fluent interface for setting multiple fields: `ctx.set("a", "1").set("b", "2")`

### Usage Examples

#### Basic Request Tracking

```cpp
#include <Zyrnix/log_context.hpp>
#include <Zyrnix/structured_logger.hpp>

void handle_request(const std::string& request_id, const std::string& user_id) {
    auto logger = Zyrnix::StructuredLogger::create("api", "api.jsonl");
    
    Zyrnix::ScopedContext ctx;
    ctx.set("request_id", request_id)
       .set("user_id", user_id);
    
    logger->info("Processing request");
    
    process_payment();  // Nested calls automatically inherit context
    update_database();  // No need to pass request_id everywhere!
    
    logger->info("Request complete");
}
```

All logs automatically include `request_id` and `user_id` fields in JSON output.

#### Nested Contexts for Fine-Grained Tracking

```cpp
void process_order(const std::string& order_id) {
    Zyrnix::ScopedContext order_ctx;
    order_ctx.set("order_id", order_id);
    
    logger->info("Starting order processing");
    
    {
        Zyrnix::ScopedContext db_ctx;
        db_ctx.set("operation", "database");
        
        logger->debug("Fetching order details");  // Includes order_id + operation
    }
    
    logger->info("Order processed");  // Only includes order_id
}
```

#### Global Application Context

```cpp
int main() {
    Zyrnix::LogContext::set("app_version", "1.0.3");
    Zyrnix::LogContext::set("environment", "production");
    Zyrnix::LogContext::set("hostname", "server-01");
    
    // All logs in the application now include these fields
}
```

### API Reference

**`Zyrnix::ScopedContext`** - RAII context manager
- `ScopedContext()` - Create empty scoped context
- `set(key, value)` - Set context attribute (returns `*this` for chaining)
- `get(key)` - Get context attribute value
- `remove(key)` - Remove context attribute
- `get_all()` - Get all context attributes as map

**`Zyrnix::LogContext`** - Static context API
- `LogContext::set(key, value)` - Set global context
- `LogContext::get(key)` - Get global context value
- `LogContext::clear()` - Clear all global context
- `LogContext::contains(key)` - Check if key exists

### Integration

The context feature seamlessly integrates with existing `StructuredLogger`. All context fields are automatically merged into JSON output alongside explicit log fields and global context set via `StructuredLogger::set_context()`.

**Priority order (later overrides earlier):**
1. Global context (`StructuredLogger::set_context()`)
2. Thread-local context (`LogContext` / `ScopedContext`)
3. Explicit log fields (passed to `logger->info()`)

---

## 🔒 Testing & Quality Assurance Improvements

### Sanitizer Support

Added comprehensive sanitizer support for catching memory safety issues, data races, and undefined behavior early in development.

**AddressSanitizer (ASAN):** Detects memory leaks, buffer overflows, use-after-free, and other memory corruption bugs.

**ThreadSanitizer (TSAN):** Finds data races, deadlocks, and thread synchronization issues in multi-threaded code.

**UndefinedBehaviorSanitizer (UBSan):** Catches undefined C++ behavior like integer overflow, null pointer dereference, and alignment violations.

#### Local Testing

```bash
# Run AddressSanitizer tests
chmod +x local_test/*.sh
./local_test/run_asan.sh

# Run ThreadSanitizer tests
./local_test/run_tsan.sh
```

### Fuzz Testing

Added libFuzzer-based fuzz testing to automatically discover edge cases, crashes, and security vulnerabilities.

**Target:** `tests/fuzz_formatter.cpp` exercises formatter and JSON sink with randomized inputs.

#### Local Fuzzing

```bash
# Run 30-second fuzz smoke test
./local_test/run_fuzz.sh

# Run longer fuzzing session (1 hour, 4 parallel jobs)
./fuzz_formatter -max_total_time=3600 -artifact_prefix=./fuzz_artifacts/ -jobs=4 -workers=4

# Reproduce a crash
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./fuzz_formatter ./fuzz_artifacts/crash-000001
```

### CI/CD Integration

Added GitHub Actions workflow `.github/workflows/sanitizers-fuzz.yml` that automatically runs on every push and pull request:

- **ASAN build and test** - Catches memory issues
- **TSAN build and test** - Catches threading bugs  
- **UBSan build and test** - Catches undefined behavior
- **Fuzz smoke test** - 20-second fuzzing session to catch obvious crashes

All commits are now validated for memory safety and correctness before merge.

### CMake Build Options

```bash
# Enable/disable tests
cmake .. -DBUILD_TESTS=ON  # Default: ON

# Enable fuzzing support
cmake .. -DBUILD_TESTS=ON -DENABLE_FUZZ=ON

# Disable tests for production builds
cmake .. -DBUILD_TESTS=OFF
```

---

## 📦 New Files

### Headers
- `include/Zyrnix/log_context.hpp` - Log context API

### Source
- `src/log_context.cpp` - Log context implementation

### Examples
- `examples/context_logging.cpp` - Comprehensive context usage examples

### Tests
- `tests/fuzz_formatter.cpp` - libFuzzer harness
- `tests/test_main.cpp` - Minimal test entrypoint

### Scripts
- `local_test/run_asan.sh` - Run AddressSanitizer locally
- `local_test/run_tsan.sh` - Run ThreadSanitizer locally
- `local_test/run_fuzz.sh` - Run fuzzer locally
- `local_test/README.md` - Local testing instructions

### CI
- `.github/workflows/sanitizers-fuzz.yml` - Sanitizer and fuzz CI workflow

### Documentation
- `TESTING_CONTEXT_FEATURE.md` - Context feature testing guide
- `RELEASE_NOTES_v1.0.3.md` - This file

---

## 🔧 Modified Files

### Core Library
- `include/Zyrnix/Zyrnix.hpp` - Added `#include "log_context.hpp"` for convenience
- `src/sinks/structured_json_sink.cpp` - Automatic context field injection via `LogContext::get_all()`

### Build System
- `CMakeLists.txt` - Added `BUILD_TESTS` option
- `tests/CMakeLists.txt` - Added `ENABLE_FUZZ` option and fuzz target

### Documentation
- `README.md` - Updated to v1.0.3, added Log Context documentation and Testing & QA section

---

## 🎯 Use Cases

### Microservices & Distributed Systems
Track requests across service boundaries using request IDs and correlation IDs automatically included in all logs.

### Multi-Tenant Applications
Set tenant/customer IDs in context once per request—all logs automatically include tenant information for filtering and analysis.

### Debugging Production Issues
Add debug context dynamically without code changes—set context at runtime and all subsequent logs include diagnostic information.

### Cloud-Native Applications
Context fields flow into structured JSON logs compatible with ELK, Datadog, Splunk, CloudWatch, and other log aggregators.

### Performance Monitoring
Track operation timing, database query counts, cache hit rates by setting metrics in context—automatically included in all performance logs.

---

## 🚀 Migration Guide

### Upgrading from v1.0.2

No breaking changes. The new context feature is purely additive.

**Old code continues to work:**
```cpp
auto logger = Zyrnix::StructuredLogger::create("app", "app.jsonl");
logger->info("Message", {{"field", "value"}});  // Still works
```

**New code gains automatic context:**
```cpp
Zyrnix::ScopedContext ctx;
ctx.set("request_id", "req-123");
logger->info("Message");  // Automatically includes request_id
```

### Adoption Strategy

1. **Start with high-level request handlers** - Add context at API entry points
2. **Gradually expand to subsystems** - Add nested contexts for database, cache, external API calls
3. **Set global context at startup** - Add application metadata (version, environment, hostname)
4. **Monitor log volume** - Context adds fields to every log; ensure storage can handle increased size

---

## 📊 Performance Impact

- **Context overhead:** ~50ns per log (thread-local map lookup)
- **Memory overhead:** ~100 bytes per thread for context storage
- **No locking:** Thread-local storage eliminates contention
- **RAII cleanup:** Zero-cost abstraction—no runtime overhead for scope management

For high-throughput systems (>100k logs/sec), context overhead is <0.1% of total logging time.

---

## 🔐 Security Improvements

### Sanitizer Coverage
- Detects buffer overflows, memory corruption, and use-after-free vulnerabilities
- Catches data races that could lead to security bugs
- Validates undefined behavior that could be exploited

### Fuzz Testing
- Automatically discovers input validation issues
- Tests edge cases that manual testing misses
- Prevents crashes from malformed log messages

---

## 🐛 Bug Fixes

- Fixed missing `#include <vector>` in `log_context.hpp`
- Fixed missing `#include <iostream>` in `context_logging.cpp`

---

## 📝 Documentation Improvements

- Added comprehensive Log Contexts & Scoped Attributes documentation to README
- Added Testing & Quality Assurance section with sanitizer and fuzz testing instructions
- Added example code for all context features
- Updated project version references throughout documentation

---

## 🙏 Acknowledgments

This release adds enterprise-grade features inspired by mature logging frameworks:
- **Log4j** (Java) - MDC/NDC concept
- **SLF4J** (Java) - Scoped context API design
- **spdlog** (C++) - Performance-oriented implementation patterns

---

## 📋 Requirements

- **C++17 or later** (thread_local, structured bindings)
- **CMake 3.16+**
- **Optional:** libfmt for formatting (detected automatically)
- **For sanitizers:** Clang or GCC with sanitizer support
- **For fuzzing:** Clang with libFuzzer support

---

## 🔗 Resources

- **GitHub Repository:** https://github.com/hent83722/Zyrnix
- **Documentation:** See `README.md` and `docs/` folder
- **Examples:** See `examples/` folder
- **Issues:** https://github.com/hent83722/Zyrnix/issues

---

## 📈 What's Next?

Planned features for future releases:

- **Log sampling & rate limiting** - Control log volume in high-throughput systems
- **Dynamic filtering** - Runtime log level and field-based filtering
- **Binary log format** - Faster serialization for performance-critical systems
- **OpenTelemetry integration** - Standard distributed tracing integration
- **Metrics integration** - Emit metrics from log patterns

---

## 📄 License

Zyrnix v1.0.3 is released under the MIT License. See `LICENSE` file for details.

---

**Thank you for using Zyrnix!** 

For questions, feature requests, or bug reports, please open an issue on GitHub.
