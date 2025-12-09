<div align="center">

I appreciate you giving this project a ⭐ :)

# 🚀 XLog

### Modern High-Performance C++ Logging Library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Version](https://img.shields.io/badge/version-1.0.4-brightgreen.svg)](https://github.com/hent83722/xlog/releases)
[![CI](https://img.shields.io/badge/CI-passing-success.svg)](https://github.com/hent83722/xlog/actions)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)]()

**Production-ready, thread-safe logging with structured output, async support, and zero-overhead abstractions**

[Features](#-features) • [Quick Start](#-quick-start) • [Documentation](#-documentation) • [Examples](#-examples) • [Contributing](#-contributing)

</div>

---

## 📋 Overview

**XLog** is a modern, lightweight, and blazingly fast logging library for C++17+. Inspired by industry-standard loggers like `spdlog` and `log4j`, XLog combines elegant API design with high performance, making it perfect for everything from hobby projects to enterprise applications.

### Why XLog?

- ⚡ **Zero-cost abstractions** - Compile-time optimizations eliminate runtime overhead
- 🔒 **Thread-safe by design** - Production-grade synchronization and async logging
- 🎯 **Structured logging** - First-class JSON support for modern observability stacks
- 🌊 **Multiple sinks** - Write to console, files, syslog, network, or custom destinations
- 🧪 **Battle-tested** - AddressSanitizer, ThreadSanitizer, UndefinedBehaviorSanitizer, and fuzz tested
- 📦 **Easy integration** - Header-only or static library, minimal dependencies

---

## ✨ Features

<table>
<tr>
<td width="50%">

### Core Features
- ✅ Multiple log levels (Trace, Debug, Info, Warn, Error, Critical)
- ✅ Stream-style and printf-style syntax
- ✅ Header-only or compiled library modes
- ✅ Thread-safe synchronous logging
- ✅ High-performance asynchronous logging
- ✅ Compile-time and runtime filtering

</td>
<td width="50%">

### Advanced Features
- ✅ **Structured JSON logging** for cloud platforms
- ✅ **Log contexts (MDC/NDC)** for request tracking
- ✅ Rotating, daily, and size-based file sinks
- ✅ Network sinks (UDP, Syslog)
- ✅ Custom formatters and sinks
- ✅ Color-coded console output

</td>
</tr>
</table>

---

## 🚀 Quick Start

### Installation

**Option 1: Quick Install (Linux/macOS)**

```bash
git clone https://github.com/hent83722/xlog.git
cd xlog
bash scripts/build.sh && sudo bash scripts/install.sh
```

**Option 2: CMake Integration**

```bash
# Clone and build
git clone https://github.com/hent83722/xlog.git
cd xlog && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
sudo cmake --install .
```

### Your First Logger

```cpp
#include <xlog/xlog.hpp>

int main() {
    // Create a logger
    auto logger = xlog::Logger::create_stdout_logger("app");
    
    // Log messages
    logger->log(xlog::LogLevel::Info, "Application started");
    
    // Stream-style logging
    *logger << xlog::Info << "User logged in: " << "john@example.com" << xlog::endl;
    
    return 0;
}
```

**Compile:**
```bash
g++ -std=c++17 main.cpp -lxlog -o myapp
```

---

## 📖 Documentation

### Installation Methods

#### Option 1: Using Helper Scripts (Recommended for Linux/macOS)

**Build the library:**
```bash
bash scripts/build.sh
```
- Builds XLog in Release mode
- Generates `libxlog.a` static library

**Install system-wide:**
```bash 
sudo bash scripts/install.sh
```
- Installs to `/usr/local/lib` and `/usr/local/include/xlog`

**Run tests:**
```bash
bash scripts/debug_run.sh      # Run tests in debug mode
bash scripts/memcheck.sh       # Valgrind memory check
```

**Code quality:**
```bash 
bash scripts/format.sh         # Format code with clang-format
bash scripts/tidy.sh          # Static analysis with clang-tidy
```

#### Option 2: Manual CMake (Cross-platform)

**Clone and build:**
```bash 
git clone https://github.com/hent83722/xlog.git
cd xlog && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

**Install:**
```bash
sudo cmake --install .
```

**Integrate with your CMake project:**
```cmake
find_package(xlog REQUIRED)
target_link_libraries(your_project PRIVATE xlog)
```

Or as a subdirectory:
```cmake
add_subdirectory(external/xlog)
target_link_libraries(your_project PRIVATE xlog)
```

---

## 💡 Usage Examples

### Basic Logging

```cpp
#include <xlog/xlog.hpp>

auto logger = xlog::Logger::create_stdout_logger("app");

// Different log levels
logger->log(xlog::LogLevel::Trace, "Detailed trace information");
logger->log(xlog::LogLevel::Info, "Application started");
logger->log(xlog::LogLevel::Warn, "Configuration file not found");
logger->log(xlog::LogLevel::Error, "Failed to connect to database");

// Stream-style logging
*logger << xlog::Info << "User count: " << 42 << xlog::endl;
```

### Multiple Sinks

Write logs to multiple destinations simultaneously:

```cpp
#include <xlog/logger.hpp>
#include <xlog/sinks/file_sink.hpp>
#include <xlog/sinks/stdout_sink.hpp>

auto logger = std::make_shared<xlog::Logger>("multi");
logger->add_sink(std::make_shared<xlog::FileSink>("app.log"));
logger->add_sink(std::make_shared<xlog::StdoutSink>());

logger->log(xlog::LogLevel::Info, "Logged to both file and console");
```

### Asynchronous Logging

High-performance async logging for production systems:

```cpp
auto async_logger = xlog::Logger::create_async("async");
async_logger->log(xlog::LogLevel::Info, "Non-blocking log message");
```

### Rotating File Logs

Automatically rotate logs based on file size:

```cpp
#include <xlog/sinks/rotating_file_sink.hpp>

auto logger = std::make_shared<xlog::Logger>("rotating");
// Rotate every 10MB, keep 5 files
logger->add_sink(std::make_shared<xlog::RotatingFileSink>(
    "app.log", 10 * 1024 * 1024, 5
));
```

---

## 🎯 Advanced Features

### Structured JSON Logging
Perfect for cloud-native applications and log aggregators (ELK, Datadog, Splunk, CloudWatch):

```cpp
#include <xlog/structured_logger.hpp>

auto slog = xlog::StructuredLogger::create("api", "app.jsonl");

// Set global context
slog->set_context("request_id", "req-12345");
slog->set_context("service", "user-api");

// Log with structured fields
slog->info("User login successful", {
    {"user_id", "user-456"},
    {"duration_ms", "145"},
    {"ip_address", "192.168.1.100"}
});
```

**Output (JSON Lines format):**
```json
{"timestamp":"2025-12-07T14:54:55.714Z","level":"INFO","logger":"api","message":"User login successful","request_id":"req-12345","service":"user-api","user_id":"user-456","duration_ms":"145","ip_address":"192.168.1.100"}
```

**Benefits:**
- ✅ Cloud-ready JSON Lines format
- ✅ Queryable structured fields
- ✅ Distributed tracing support
- ✅ Easy integration with observability platforms

### Log Contexts & Request Tracking

Track request IDs, user sessions, and transactions across your entire call stack **without passing parameters everywhere**.

XLog provides Mapped Diagnostic Context (MDC) functionality similar to Log4j and SLF4J. Context attributes are stored thread-locally and automatically included in all log messages.

**Basic usage:**

```cpp
#include <xlog/log_context.hpp>
#include <xlog/structured_logger.hpp>

void process_order(const std::string& order_id) {
    auto logger = xlog::StructuredLogger::create("orders", "orders.jsonl");
    
    // Scoped context - auto cleanup on scope exit
    xlog::ScopedContext ctx;
    ctx.set("order_id", order_id).set("user_id", "user-123");
    
    // All logs automatically include order_id and user_id
    logger->info("Processing order");
    validate_payment();   // Nested calls inherit context
    update_inventory();   // No parameter passing needed!
} // Context automatically cleared
```

**HTTP request tracking:**
```cpp
void handle_request(const HttpRequest& req) {
    xlog::ScopedContext ctx;
    ctx.set("request_id", req.header("X-Request-ID"))
       .set("user_id", req.user_id())
       .set("endpoint", req.path());
    
    // All logs in this scope include request metadata
    logger->info("Request received");
    process_business_logic();
    logger->info("Request completed");
}
```

---

## 🔌 Network Sinks & Production Integration

Integrate XLog into production systems with multiple output destinations:

```cpp
#include <xlog/logger.hpp>
#include <xlog/sinks/file_sink.hpp>
#include <xlog/sinks/syslog_sink.hpp>
#include <xlog/sinks/udp_sink.hpp>

auto logger = std::make_shared<xlog::Logger>("production");

// Local file
logger->add_sink(std::make_shared<xlog::FileSink>("/var/log/app.log"));

// System syslog (Linux/macOS)
logger->add_sink(std::make_shared<xlog::SyslogSink>("myapp", LOG_PID, LOG_USER));

// Remote log collector
logger->add_sink(std::make_shared<xlog::UdpSink>("logs.company.com", 514));
```

**Integration patterns:**
- 🎯 **Per-subsystem loggers** - Create separate loggers for HTTP, DB, auth, etc.
- 🌍 **Environment-specific sinks** - Console in dev, syslog/network in production
- 📊 **Structured outputs** - Use JSON for log aggregators and SIEM platforms
- ⚡ **Async network sinks** - Non-blocking to avoid latency impact

---

## 🧪 Testing & Quality Assurance

XLog is battle-tested with comprehensive quality assurance:

### 🔍 Sanitizer Coverage

```bash
# AddressSanitizer - memory leaks, buffer overflows
./local_test/run_asan.sh

# ThreadSanitizer - data races, deadlocks  
./local_test/run_tsan.sh

# UndefinedBehaviorSanitizer - undefined behavior
./local_test/run_ubsan.sh
```

### 🎲 Fuzz Testing

```bash
# Quick fuzz test (30 seconds)
./local_test/run_fuzz.sh

# Extended fuzzing (1 hour, 4 workers)
./fuzz_formatter -max_total_time=3600 -jobs=4 -workers=4
```

### 🤖 CI/CD

Every commit automatically runs:
- ✅ AddressSanitizer
- ✅ ThreadSanitizer  
- ✅ UndefinedBehaviorSanitizer
- ✅ Fuzz testing (20s smoke test)
- ✅ Unit tests across multiple platforms

**Production-ready quality, guaranteed.**

---

## 📚 Examples

Explore complete working examples in the `examples/` directory:

| Example | Description |
|---------|-------------|
| `basic_logging.cpp` | Simple console logging |
| `async_logging.cpp` | High-performance async logging |
| `multi_sink_example.cpp` | Write to multiple destinations |
| `rotating_logs.cpp` | Rotating file logs for production |
| `structured_json_example.cpp` | JSON logging for cloud platforms |
| `context_logging.cpp` | Request tracking with MDC |
| `network_syslog_example.cpp` | Remote syslog integration |
| `custom_sink.cpp` | Implement custom log destinations |

---

## 🏗️ Project Structure

```
xlog/
├── include/xlog/          # Public API headers
├── src/                   # Implementation files
├── examples/              # Complete usage examples
├── tests/                 # Unit tests
├── benchmarks/            # Performance benchmarks
├── docs/                  # Documentation
└── scripts/               # Build and test scripts
```

---

## ❓ Why XLog Over `std::cout`?

| Feature | `std::cout` | XLog |
|---------|-------------|------|
| **Log Levels** | ❌ Manual | ✅ Built-in (Trace, Debug, Info, Warn, Error, Critical) |
| **Multiple Outputs** | ❌ Redirect only | ✅ Multiple sinks simultaneously |
| **Thread Safety** | ⚠️ Garbled output | ✅ Synchronized and lock-free modes |
| **Structured Logging** | ❌ Text only | ✅ JSON, key-value pairs |
| **Async Performance** | ❌ Blocking I/O | ✅ Lock-free async logging |
| **Filtering** | ❌ Manual | ✅ Compile-time and runtime |
| **Production Ready** | ❌ Debug tool | ✅ Enterprise-grade |

---

## 🤝 Contributing
We welcome contributions! Here's how to get started:

1. 🍴 Fork the repository
2. 🌿 Create a feature branch (`git checkout -b feature/amazing-feature`)
3. ✅ Make your changes with tests
4. 🔍 Run sanitizers and tests (`./scripts/test_all_features.sh`)
5. 📝 Commit your changes (`git commit -m 'Add amazing feature'`)
6. 🚀 Push to your branch (`git push origin feature/amazing-feature`)
7. 🎯 Open a Pull Request

**Areas we'd love help with:**
- New sink implementations (Kafka, Redis, etc.)
- Performance optimizations
- Documentation improvements
- Platform-specific fixes (Windows, BSD)
- More examples and use cases

---

## 📄 License

XLog is released under the [MIT License](LICENSE).

```
Copyright (c) 2025 XLog Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files...
```

---

## 🙏 Acknowledgments

XLog is inspired by:
- [spdlog](https://github.com/gabime/spdlog) - Fast C++ logging library
- [log4j](https://logging.apache.org/log4j/) - Java logging framework
- [serilog](https://serilog.net/) - Structured logging for .NET

---

## 🔗 Links

- 📖 [Full Documentation](docs/)
- 🐛 [Report Issues](https://github.com/hent83722/xlog/issues)
- 💬 [Discussions](https://github.com/hent83722/xlog/discussions)
- 📦 [Releases](https://github.com/hent83722/xlog/releases)

---

<div align="center">

**Built with ❤️ for the C++ community**

If XLog helps your project, consider giving it a ⭐!

[⬆ Back to Top](#-xlog)

</div>
