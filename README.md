<div align="center">


# 🚀 Zyrnix
  
### Modern High-Performance C++ Logging Library
  
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Version](https://img.shields.io/badge/version-1.1.2-brightgreen.svg)](https://github.com/hent83722/Zyrnix/releases)
[![CI](https://img.shields.io/badge/CI-passing-success.svg)](https://github.com/hent83722/Zyrnix/actions)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)]()

**Production-ready, thread-safe logging with structured output, async support, and zero-overhead abstractions**

[Features](#-features) • [Quick Start](#-quick-start) • [Documentation](#-documentation) • [Examples](#-examples) • [Contributing](#-contributing)

</div>

---

## 📋 Overview

**Zyrnix** is a modern, lightweight, and blazingly fast logging library for C++17+. Inspired by industry-standard loggers like `spdlog` and `log4j`, Zyrnix combines elegant API design with high performance, making it perfect for everything from hobby projects to enterprise applications.

### Why Zyrnix?

- ⚡ **Zero-cost abstractions** - Compile-time optimizations eliminate runtime overhead
- 🔒 **Thread-safe by design** - Production-grade synchronization and async logging
- 🎯 **Structured logging** - First-class JSON support for modern observability stacks
- 🌊 **Multiple sinks** - Write to console, files, syslog, network, or custom destinations
- 🧪 **Battle-tested** - AddressSanitizer, ThreadSanitizer, UndefinedBehaviorSanitizer, and fuzz tested
- 📦 **Easy integration** - Header-only or static library, minimal dependencies
- 🏥 **Production-ready** - Health checks, dynamic config, and observability built-in
- 🎚️ **Adaptive performance** - Auto-tuning compression and intelligent rate limiting

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

### Advanced Features (v1.0.4)
- ✅ **Structured JSON logging** for cloud platforms
- ✅ **Log contexts (MDC/NDC)** for request tracking
- ✅ **Configuration files** - JSON config without recompiling
- ✅ **Signal-safe logging** - Crash handler support
- ✅ **Conditional compilation** - Reduce binary size 50-70KB
- ✅ Rotating, daily, and size-based file sinks
- ✅ Network sinks (UDP, Syslog)
- ✅ Custom formatters and sinks
- ✅ Color-coded console output

</td>
</tr>
<tr>
<td colspan="2">

### 🆕 Enterprise Features (v1.1.0)
- 🚀 **Rate Limiting & Sampling** - Prevent log flooding with token bucket algorithm
- 💾 **Compression Support** - Save 70-90% disk space with gzip/zstd compression
- ☁️ **Cloud Sinks** - Native AWS CloudWatch & Azure Monitor integration
- 📊 **Metrics & Observability** - Built-in telemetry with Prometheus export

[📖 See v1.1.0 Features Documentation →](docs/v1.1.0_FEATURES.md)

</td>
</tr>
<tr>
<td colspan="2">

### 🔥 Latest Features (v1.1.1)
- 🎯 **Regex Log Filtering** - Pattern-based filtering with invert support
- 🔄 **Dynamic Log Levels** - Thread-safe runtime level changes with callbacks
- 🏥 **Health Check API** - Monitor logger health for SRE/DevOps integration
- 🎚️ **Compression Auto-Tune** - Adaptive compression level optimization

[📖 See v1.1.1 Release Notes →](docs/notes/RELEASE_NOTES_v1.1.1-beta.1.md)

</td>
</tr>
<tr>
<td colspan="2">

### 🛠️ Latest: Bug Fixes & Hardening (v1.1.2)
- 🔒 **Flush Guarantees on Shutdown** - Async queues drain completely with configurable timeout
- 🔄 **Thread-Safe Sink Removal** - Hot-remove sinks without blocking writers (reference counting)
- 🪟 **Windows Unicode Path Support** - Proper UTF-8 → UTF-16 conversion for international paths
- ⚡ **Signal Handler Reentrancy Fixes** - Audited signal-safe sink for maximum reliability

[📖 See v1.1.2 Release Notes →](docs/notes/RELEASE_NOTES_v1.1.2.md)

</td>
</tr>
</table>

---

## 🚀 Quick Start

### Installation

Zyrnix can be installed using the provided installation scripts (recommended) or manually via CMake.

<details>
<summary><b>🔧 Option 1: Installation Scripts (Recommended)</b></summary>

We provide platform-specific scripts that handle the entire build and install process.

**Linux:**
```bash
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix
./scripts/install_linux.sh
```

**macOS:**
```bash
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix
./scripts/install_mac.sh
```

**Windows (PowerShell - Run as Administrator):**
```powershell
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix
.\scripts\install_windows.ps1
```

**Windows (Command Prompt - Run as Administrator):**
```cmd
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix
scripts\install_windows.bat
```

> 💡 **Script Options:** All scripts support `--help` for available options like `--debug`, `--prefix=PATH`, and `--jobs=N`.

</details>

<details>
<summary><b>⚙️ Option 2: Manual CMake Installation</b></summary>

**Linux/macOS:**
```bash
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
sudo cmake --install .
```

**Windows (from Developer PowerShell/Command Prompt):**
```powershell
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel
cmake --install . --config Release
```

> 📝 **Note:** On Windows, run as Administrator for system-wide installation, or specify a custom prefix: `cmake .. -DCMAKE_INSTALL_PREFIX="C:\Users\you\Zyrnix"`

</details>

### Your First Logger

```cpp
#include <Zyrnix/Zyrnix.hpp>

int main() {
    // Create a logger
    auto logger = Zyrnix::Logger::create_stdout_logger("app");
    
    // Log messages
    logger->log(Zyrnix::LogLevel::Info, "Application started");
    
    // Stream-style logging
    *logger << Zyrnix::Info << "User logged in: " << "john@example.com" << Zyrnix::endl;
    
    return 0;
}
```

**Compile:**
```bash
g++ -std=c++17 main.cpp -lZyrnix -o myapp
```

---

## 📖 Documentation

### Installation Methods

#### 🔧 Using Installation Scripts (Recommended)

Installation scripts automate the build and system installation process. They check for dependencies, build in Release mode, and install to standard system locations.

<table>
<tr>
<th>Platform</th>
<th>Script</th>
<th>Command</th>
</tr>
<tr>
<td>🐧 Linux</td>
<td><code>install_linux.sh</code></td>
<td>

```bash
./scripts/install_linux.sh
```
</td>
</tr>
<tr>
<td>🍎 macOS</td>
<td><code>install_mac.sh</code></td>
<td>

```bash
./scripts/install_mac.sh
```
</td>
</tr>
<tr>
<td>🪟 Windows</td>
<td><code>install_windows.ps1</code></td>
<td>

```powershell
.\scripts\install_windows.ps1
```
</td>
</tr>
<tr>
<td>🪟 Windows (CMD)</td>
<td><code>install_windows.bat</code></td>
<td>

```cmd
scripts\install_windows.bat
```
</td>
</tr>
</table>

**Script Options:**

| Option | Description | Default |
|--------|-------------|--------|
| `--debug` | Build in Debug mode | Release |
| `--prefix=PATH` | Custom installation directory | `/usr/local` (Linux/macOS), `C:\Program Files\Zyrnix` (Windows) |
| `--jobs=N` | Parallel build jobs | Auto-detected |
| `--help` | Show all available options | - |

**Example with options:**
```bash
# Linux/macOS - Install to custom location
./scripts/install_linux.sh --prefix=/opt/Zyrnix --jobs=8

# Windows PowerShell - Debug build to custom location
.\scripts\install_windows.ps1 -BuildType Debug -InstallPrefix "C:\dev\Zyrnix"
```

---

#### ⚙️ Manual CMake Installation (Cross-platform)

For full control over the build process, you can build and install manually using CMake.

**Linux/macOS:**
```bash
# Clone the repository
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --parallel

# Install (requires sudo)
sudo cmake --install .
```

**Windows (Visual Studio):**
```powershell
# Clone the repository
git clone https://github.com/hent83722/Zyrnix.git
cd Zyrnix

# Create build directory
mkdir build && cd build

# Configure (uses default Visual Studio generator)
cmake ..

# Build
cmake --build . --config Release --parallel

# Install (run as Administrator)
cmake --install . --config Release
```

**Custom Install Location:**
```bash
# Linux/macOS
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/Zyrnix

# Windows
cmake .. -DCMAKE_INSTALL_PREFIX="C:\Libraries\Zyrnix"
```

---

#### 📦 Integrating with Your CMake Project

**After system installation:**
```cmake
find_package(Zyrnix REQUIRED)
target_link_libraries(your_project PRIVATE Zyrnix::Zyrnix)
```

**As a subdirectory (no installation needed):**
```cmake
add_subdirectory(external/Zyrnix)
target_link_libraries(your_project PRIVATE Zyrnix)
```

**Windows - specify install location if needed:**
```cmake
set(CMAKE_PREFIX_PATH "C:/Program Files/Zyrnix")
find_package(Zyrnix REQUIRED)
target_link_libraries(your_project PRIVATE Zyrnix::Zyrnix)
```

---

#### 🛠️ Development Scripts

| Script | Description |
|--------|-------------|
| `scripts/build.sh` | Build the library (Release mode) |
| `scripts/debug_run.sh` | Run tests in debug mode |
| `scripts/memcheck.sh` | Valgrind memory check |
| `scripts/format.sh` | Format code with clang-format |
| `scripts/tidy.sh` | Static analysis with clang-tidy |

---

## 💡 Usage Examples

### Basic Logging

```cpp
#include <Zyrnix/Zyrnix.hpp>

auto logger = Zyrnix::Logger::create_stdout_logger("app");

// Different log levels
logger->log(Zyrnix::LogLevel::Trace, "Detailed trace information");
logger->log(Zyrnix::LogLevel::Info, "Application started");
logger->log(Zyrnix::LogLevel::Warn, "Configuration file not found");
logger->log(Zyrnix::LogLevel::Error, "Failed to connect to database");

// Stream-style logging
*logger << Zyrnix::Info << "User count: " << 42 << Zyrnix::endl;
```

### Multiple Sinks

Write logs to multiple destinations simultaneously:

```cpp
#include <Zyrnix/logger.hpp>
#include <Zyrnix/sinks/file_sink.hpp>
#include <Zyrnix/sinks/stdout_sink.hpp>

auto logger = std::make_shared<Zyrnix::Logger>("multi");
logger->add_sink(std::make_shared<Zyrnix::FileSink>("app.log"));
logger->add_sink(std::make_shared<Zyrnix::StdoutSink>());

logger->log(Zyrnix::LogLevel::Info, "Logged to both file and console");
```

### Asynchronous Logging

High-performance async logging for production systems:

```cpp
auto async_logger = Zyrnix::Logger::create_async("async");
async_logger->log(Zyrnix::LogLevel::Info, "Non-blocking log message");
```

### Rotating File Logs

Automatically rotate logs based on file size:

```cpp
#include <Zyrnix/sinks/rotating_file_sink.hpp>

auto logger = std::make_shared<Zyrnix::Logger>("rotating");
// Rotate every 10MB, keep 5 files
logger->add_sink(std::make_shared<Zyrnix::RotatingFileSink>(
    "app.log", 10 * 1024 * 1024, 5
));
```

---

## 🎯 Advanced Features

### Structured JSON Logging
Perfect for cloud-native applications and log aggregators (ELK, Datadog, Splunk, CloudWatch):

```cpp
#include <Zyrnix/structured_logger.hpp>

auto slog = Zyrnix::StructuredLogger::create("api", "app.jsonl");

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

Zyrnix provides Mapped Diagnostic Context (MDC) functionality similar to Log4j and SLF4J. Context attributes are stored thread-locally and automatically included in all log messages.

**Basic usage:**

```cpp
#include <Zyrnix/log_context.hpp>
#include <Zyrnix/structured_logger.hpp>

void process_order(const std::string& order_id) {
    auto logger = Zyrnix::StructuredLogger::create("orders", "orders.jsonl");
    
    // Scoped context - auto cleanup on scope exit
    Zyrnix::ScopedContext ctx;
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
    Zyrnix::ScopedContext ctx;
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

## 🆕 What's New in v1.0.4

### 📄 Configuration File Support

Load logger configurations from JSON files without recompiling:

```cpp
#include <Zyrnix/config.hpp>

// Load from JSON file
Zyrnix::ConfigLoader::load_from_json("config.json");
auto loggers = Zyrnix::ConfigLoader::create_loggers();

// Use configured loggers
auto app_logger = loggers["app"];
app_logger->info("Configuration loaded dynamically!");
```

**Example config.json:**
```json
{
````

---

## 🔥 What's New in v1.1.1

### 🎯 Regex-Based Log Filtering

Filter logs using powerful regular expressions:

```cpp
#include <Zyrnix/log_filter.hpp>

auto logger = Zyrnix::Logger::create_stdout_logger("app");

// Only log messages containing ERROR or CRITICAL
auto error_filter = std::make_shared<Zyrnix::RegexFilter>("(ERROR|CRITICAL)");
logger->add_filter(error_filter);

// Exclude sensitive data (inverted match - logs everything EXCEPT matches)
auto no_secrets = std::make_shared<Zyrnix::RegexFilter>("(password|token|secret)", true);
logger->add_filter(no_secrets);
```

### 🔄 Dynamic Log Level Changes

Change log levels at runtime without restarting:

```cpp
auto logger = Zyrnix::Logger::create_stdout_logger("app");

// Register callback for level changes
logger->register_level_change_callback([](LogLevel old_lvl, LogLevel new_lvl) {
    std::cout << "Log level changed!" << std::endl;
});

// Thread-safe level change (great for config hot-reload)
logger->set_level_dynamic(Zyrnix::LogLevel::Debug);
```

### 🏥 Health Check API

Monitor your logging infrastructure:

```cpp
#include <Zyrnix/log_health.hpp>

// Register logger for monitoring
Zyrnix::HealthRegistry::instance().register_logger("api", logger);

// Check health (perfect for K8s probes)
auto result = Zyrnix::HealthRegistry::instance().check_logger("api");

if (Zyrnix::HealthChecker::is_healthy(result)) {
    // All good!
} else {
    std::cerr << result.to_json() << std::endl;  // Export for monitoring
}
```

### 🎚️ Compression Auto-Tune

Automatic compression level optimization:

```cpp
#include <Zyrnix/sinks/compressed_file_sink.hpp>

Zyrnix::CompressionOptions options;
options.type = Zyrnix::CompressionType::Gzip;
options.auto_tune = true;  // Enable auto-tune!

auto sink = std::make_shared<Zyrnix::CompressedFileSink>(
    "app.log", 10*1024*1024, 5, options
);

// Compression level adjusts automatically based on:
// - Compression ratio achieved
// - Compression speed
std::cout << "Current level: " << sink->get_current_compression_level() << std::endl;
```
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

**Benefits:**
- 🔄 Change log levels without recompiling
- 🎯 Different configs for dev/staging/production
- 📊 Dynamic A/B testing of logging strategies

### 🚨 Signal-Safe Logging for Crash Handlers

Log from signal handlers (SIGSEGV, SIGABRT, etc.) safely:

```cpp
#include <Zyrnix/sinks/signal_safe_sink.hpp>

// Set up crash logger
auto crash_sink = std::make_shared<Zyrnix::SignalSafeSink>("crash.log");
auto crash_logger = std::make_shared<Zyrnix::Logger>("crash");
crash_logger->add_sink(crash_sink);

void crash_handler(int sig) {
    // Safe to call from signal handler!
    crash_logger->log(Zyrnix::LogLevel::Critical, "Caught SIGSEGV");
    crash_sink->flush();
    _exit(1);
}

signal(SIGSEGV, crash_handler);
```

**Features:**
- ✅ Uses only async-signal-safe POSIX functions
- ✅ Lock-free ring buffer (no mutexes)
- ✅ No malloc/free in signal handlers
- ✅ Guaranteed crash log capture

### 📦 Conditional Compilation for Binary Size

Reduce binary size by 50-70KB by disabling unused features:

```bash
# Minimal build (smallest binary)
cmake -DXLOG_MINIMAL=ON ..

# Custom build (disable specific features)
cmake -DXLOG_ENABLE_ASYNC=OFF -DXLOG_ENABLE_JSON=OFF ..

# Or use compile flags
g++ -DXLOG_NO_ASYNC -DXLOG_NO_JSON main.cpp -lZyrnix
```

**Feature flags:**
- `XLOG_NO_ASYNC` - Disable async logging (~15-20KB saved)
- `XLOG_NO_JSON` - Disable JSON/structured logging (~10-15KB)
- `XLOG_NO_NETWORK` - Disable network sinks (~8-12KB)
- `XLOG_NO_COLORS` - Disable color output (~2-3KB)
- `XLOG_NO_FILE_ROTATION` - Disable rotating files (~5-8KB)
- `XLOG_NO_CONTEXT` - Disable log contexts (~3-5KB)
- `XLOG_NO_FILTERS` - Disable filtering (~2-4KB)
- `XLOG_MINIMAL` - Disable all optional features (~50-70KB)

**Feature detection in code:**
```cpp
#if XLOG_HAS_ASYNC
    auto logger = Zyrnix::Logger::create_async("app");
#else
    auto logger = Zyrnix::Logger::create_stdout_logger("app");
#endif
```

---

## 🚀 What's New in v1.1.0

Zyrnix v1.1.0 introduces four major enterprise-grade features designed for production environments:

### 1. 🎯 Rate Limiting & Sampling

**Prevent log flooding during incidents:**

```cpp
#include <Zyrnix/rate_limiter.hpp>

// Token bucket: 100 messages/sec, burst capacity 200
Zyrnix::RateLimiter limiter(100, 200);

for (int i = 0; i < 10000; ++i) {
    if (limiter.try_log()) {
        logger->error("Database connection failed");
    }
}

std::cout << "Dropped: " << limiter.dropped_count() << " messages\n";
```

**Benefits:**
- 🛡️ Prevent disk exhaustion during error storms
- ⚡ Token bucket algorithm allows controlled bursts
- 📊 Sampling support (log every Nth message)
- 📈 Built-in statistics tracking

### 2. 💾 Compression Support

**Save 70-90% disk space automatically:**

```cpp
#include <Zyrnix/sinks/compressed_file_sink.hpp>

Zyrnix::CompressionOptions options;
options.type = Zyrnix::CompressionType::Gzip;
options.level = 6;
options.compress_on_rotate = true;

auto sink = std::make_shared<Zyrnix::CompressedFileSink>(
    "app.log",
    10 * 1024 * 1024,  // 10 MB per file
    30,                // Keep 30 files
    options
);

auto logger = std::make_shared<Zyrnix::Logger>("app");
logger->add_sink(sink);

// Check compression stats
auto stats = sink->get_compression_stats();
std::cout << "Compression ratio: " << stats.compression_ratio << "x\n";
std::cout << "Space saved: " << (100.0 * (stats.original_bytes - stats.compressed_bytes) / stats.original_bytes) << "%\n";
```

**Features:**
- 🗜️ Gzip and Zstd compression support
- 🔄 Automatic compress-on-rotate
- ⚙️ Configurable compression levels (1-9 for gzip, 1-22 for zstd)
- 📊 Compression statistics tracking

### 3. ☁️ Cloud Sinks (AWS CloudWatch, Azure Monitor)

**Native cloud logging integration:**

```cpp
#include <Zyrnix/sinks/cloud_sinks.hpp>

// AWS CloudWatch
Zyrnix::CloudWatchSink::Config aws_config;
aws_config.region = "us-east-1";
aws_config.log_group_name = "/aws/myapp";
aws_config.log_stream_name = "instance-01";
aws_config.batch_size = 100;
aws_config.batch_timeout_ms = 5000;

auto cloudwatch = std::make_shared<Zyrnix::CloudWatchSink>(aws_config);

// Azure Monitor
Zyrnix::AzureMonitorSink::Config azure_config;
azure_config.instrumentation_key = "your-key";
azure_config.cloud_role_name = "my-service";
azure_config.batch_size = 100;

auto azure = std::make_shared<Zyrnix::AzureMonitorSink>(azure_config);

auto logger = std::make_shared<Zyrnix::Logger>("app");
logger->add_sink(cloudwatch);
logger->add_sink(azure);

logger->info("Logged to both AWS and Azure!");

// Monitor statistics
auto stats = cloudwatch->get_stats();
std::cout << "Messages sent: " << stats.messages_sent << "\n";
std::cout << "Batches sent: " << stats.batches_sent << "\n";
```

**Features:**
- 🌩️ AWS CloudWatch Logs integration
- ☁️ Azure Monitor / Application Insights support
- 📦 Automatic batching (reduce API costs)
- 🔁 Retry logic with exponential backoff
- ⚡ Async operation (non-blocking)
- 📊 Statistics tracking

### 4. 📊 Metrics & Observability

**Built-in telemetry for logging infrastructure:**

```cpp
#include <Zyrnix/log_metrics.hpp>

// Get metrics from global registry
auto& registry = Zyrnix::MetricsRegistry::instance();
auto metrics = registry.get_logger_metrics("app");

// Metrics are automatically tracked during logging
// Get snapshot
auto snapshot = metrics->get_snapshot();
std::cout << "Messages/sec: " << snapshot.messages_per_second << "\n";
std::cout << "Avg latency: " << snapshot.avg_log_latency_us << " µs\n";
std::cout << "Queue depth: " << snapshot.current_queue_depth << "\n";
std::cout << "Dropped: " << snapshot.messages_dropped << "\n";

// Export for Prometheus
std::string prometheus = registry.export_all_prometheus();
std::cout << prometheus;

// Or export as JSON
std::string json = registry.export_all_json();
```

**Metrics Tracked:**
- 📈 Messages per second
- ⏱️ Log latency (average, max)
- 📉 Dropped message count
- 🗄️ Queue depth (async logging)
- ❌ Error counts
- 💾 Per-sink statistics (bytes written, write latency)

**Prometheus Export:**
```
Zyrnix_messages_logged_total 125000
Zyrnix_messages_dropped_total 0
Zyrnix_messages_per_second 1234.56
Zyrnix_log_latency_us_avg 12.34
Zyrnix_queue_depth 42
```

**Perfect for:**
- Grafana dashboards
- Prometheus monitoring
- Health checks and alerting
- Performance analysis

### 📚 Complete Documentation

See [v1.1.0 Features Documentation](docs/v1.1.0_FEATURES.md) for:
- Complete API reference
- Performance characteristics
- Best practices
- Production deployment guides
- Troubleshooting

### 🎯 Example Programs

```bash
# Run examples
cd build
./examples/rate_limiting_example
./examples/compression_example
./examples/cloud_sinks_example
./examples/metrics_example
```

---

## 🔌 Network Sinks & Production Integration

Integrate Zyrnix into production systems with multiple output destinations:

```cpp
#include <Zyrnix/logger.hpp>
#include <Zyrnix/sinks/file_sink.hpp>
#include <Zyrnix/sinks/syslog_sink.hpp>
#include <Zyrnix/sinks/udp_sink.hpp>

auto logger = std::make_shared<Zyrnix::Logger>("production");

// Local file
logger->add_sink(std::make_shared<Zyrnix::FileSink>("/var/log/app.log"));

// System syslog (Linux/macOS)
logger->add_sink(std::make_shared<Zyrnix::SyslogSink>("myapp", LOG_PID, LOG_USER));

// Remote log collector
logger->add_sink(std::make_shared<Zyrnix::UdpSink>("logs.company.com", 514));
```

**Integration patterns:**
- 🎯 **Per-subsystem loggers** - Create separate loggers for HTTP, DB, auth, etc.
- 🌍 **Environment-specific sinks** - Console in dev, syslog/network in production
- 📊 **Structured outputs** - Use JSON for log aggregators and SIEM platforms
- ⚡ **Async network sinks** - Non-blocking to avoid latency impact

---

## 🧪 Testing & Quality Assurance

Zyrnix is battle-tested with comprehensive quality assurance:

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
| **`config_file_example.cpp`** | **v1.0.4: Load configs from JSON** |
| **`signal_safe_example.cpp`** | **v1.0.4: Crash handler logging** |
| **`minimal_build_example.cpp`** | **v1.0.4: Feature flags & binary size** |
| `rotating_logs.cpp` | Rotating file logs for production |
| `structured_json_example.cpp` | JSON logging for cloud platforms |
| `context_logging.cpp` | Request tracking with MDC |
| `network_syslog_example.cpp` | Remote syslog integration |
| `custom_sink.cpp` | Implement custom log destinations |

---

## 🏗️ Project Structure

```
Zyrnix/
├── include/Zyrnix/          # Public API headers
├── src/                   # Implementation files
├── examples/              # Complete usage examples
├── tests/                 # Unit tests
├── benchmarks/            # Performance benchmarks
├── docs/                  # Documentation
└── scripts/               # Build and test scripts
```

---

## ❓ Why Zyrnix Over `std::cout`?

| Feature | `std::cout` | Zyrnix |
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

Zyrnix is released under the [MIT License](LICENSE).

```
Copyright (c) 2025 Zyrnix Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files...
```

---

## 🙏 Acknowledgments

Zyrnix is inspired by:
- [spdlog](https://github.com/gabime/spdlog) - Fast C++ logging library
- [log4j](https://logging.apache.org/log4j/) - Java logging framework
- [serilog](https://serilog.net/) - Structured logging for .NET

---

## 🔗 Links

- 📖 [Full Documentation](docs/)
- 🐛 [Report Issues](https://github.com/hent83722/Zyrnix/issues)
- 💬 [Discussions](https://github.com/hent83722/Zyrnix/discussions)
- 📦 [Releases](https://github.com/hent83722/Zyrnix/releases)

---

<div align="center">

**Built with ❤️ for the C++ community**

If Zyrnix helps your project, consider giving it a ⭐!

[⬆ Back to Top](#-Zyrnix)

</div>
