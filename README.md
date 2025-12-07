# XLog - Lightweight C++ Logging Library

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
 
Latest Xlog version: v1.0.2

**XLog** is a modern, lightweight, and high-performance logging library for C++17, inspired by popular logging libraries like `spdlog`. It provides thread-safe logging, multiple sinks, log levels, and flexible formatting, making it ideal for both small projects and large-scale applications.

---



## Features

- **Header-only library** (can also compile as a static library)
- **Multiple log levels:** trace, Debug, Info, Warn, Error, Critical
- **Multiple sinks:** console, File, Rotating File, Daily File, Custom Sinks
- **Thread-safe and asynchronous logging**
- **Stream-style logging syntax** (`*logger << xlog::Info << "Message" << xlog::endl;`)
- **Flexible formatting:** timestamps, colors, structured messages, and experimental JSON support
- **Minimal dependencies:** uses standard C++17 and optional fmt library for formatting

---

## Project Structure

```text
.
├── include/xlog          # Public headers
├── src                   # Implementation files
├── examples              # Example usage
├── tests                 # Unit tests
├── benchmarks            # Performance benchmarks
├── cmake                 # Build scripts and CMake helpers
└── docs                  # Documentation and diagrams
```
## Installation & Usage

*Manual installation/usage is below using scripts area*

### Option 1: Using Scripts (Recommended, only works on linux/macos)

XLog comes with helper scripts in the ```scripts/``` folder to simplify building, installing, and testing.

1. **Build the library**
```bash
git clone https://github.com/hent83722/xlog.git
cd xlog
bash scripts/build.sh
```

* Builds XLog in ```Release``` mode in the ```build/``` folder

* Generates ```libxlog.a``` and prepares headers.

---

2. **Install system-wide**
```bash 
bash scripts/install.sh
```

* Installs the library and headers to system directories  (usually /usr/local/lib and /usr/local/include/xlog).

* Requires sudo privileges.

---

3. **Run tests or examples**

```bash
bash scripts/debug_run.sh
```

* Compiles in Debug mode and runs the test executable.

* Use ```bash scripts/memcheck.sh``` to run Valgrind and check for memory leaks.

---

4. **Format & Analyze Code**
```bash 
bash scripts/format.sh   # Format all sources and headers
bash scripts/tidy.sh     # Run static analysis with clang-tidy
```

---

## Option 2: Manual CMake Workflow (Works universally)

If you prefer not to use scripts:

1. **Clone the repository**
```bash 
git clone https://github.com/hent83722/xlog.git
cd xlog
```
---
2. **Build the library**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```
* The static library ```libxlog.a``` will be created in ```build/```.
---
3. **Install manually**
```bash
sudo cmake --install .
```
* Installs headers and library to ```/usr/local/include/```xlog and ```/usr/local/lib/.```
---
4. **Add XLog to your project**
```cmake
add_subdirectory(path/to/xlog)
target_link_libraries(your_project PRIVATE xlog)
target_include_directories(your_project PRIVATE path/to/xlog/include)
```
## Using XLog
```cpp
#include <xlog/xlog.hpp>

int main() {
    auto logger = xlog::Logger::create_stdout_logger("my_logger");
    logger->log(xlog::LogLevel::Info, "Hello, XLog!");
    return 0;
}
```
* You can also use stream-style logging:
```cpp
*logger << xlog::Info << "Stream-style logging!" << xlog::endl;
```
* Add multiple sinks:
```cpp
logger->add_sink(std::make_shared<xlog::FileSink>("app.log"));
logger->add_sink(std::make_shared<xlog::StdoutSink>());
```
* Create asynchronous loggers:
```cpp
auto async_logger = xlog::Logger::create_async("async_logger");
async_logger->log(xlog::LogLevel::Info, "Logged asynchronously!");
```
## Why Use XLog Instead of std::cout?
1. **Structured Logging**
Categorize logs by severity (Info, Warn, Error, etc.) and filter at runtime.
2. **Multiple Output Destinations**
Write logs to console, files, or custom sinks without modifying your code.
3. **Thread Safety**
Avoid garbled messages when multiple threads log simultaneously.
4. **Flexible Formatting**
Include timestamps, colors, or even JSON structures for better log analysis.
5. **Performance**
Asynchronous logging minimizes impact on your main program threads.
6. **Maintainability**
Easier debugging, clear log organization, and removal of temporary debug prints.
---
## Examples
See the examples folder for detailed use cases:
- basic_logging.cpp – Simple console logging.
- async_logging.cpp – Asynchronous logging in multi-threaded environments.
- file_vs_stdout.cpp – Logging to multiple sinks.
- rotating_logs.cpp – Rotating file logging for large projects.
- structured_json_example.cpp – Structured JSON logging for cloud platforms and log aggregators.

## Structured JSON Logging (for Cloud & Enterprise)

For cloud-native applications and integration with log aggregators (ELK, Datadog, Splunk, Cloudwatch), use `StructuredLogger`:

```cpp
#include <xlog/structured_logger.hpp>

// Create a structured logger that outputs JSON to a file
auto slog = xlog::StructuredLogger::create("api_server", "app.jsonl");

// Set global context (request ID, user ID, environment, etc.)
slog->set_context("request_id", "req-12345");
slog->set_context("service", "user-api");

// Log with additional structured fields
slog->info("User login successful", {
    {"user_id", "user-456"},
    {"duration_ms", "145"},
    {"ip_address", "192.168.1.100"}
});
```

Output (JSON Lines format, one JSON object per line):
```json
{"timestamp":"2025-12-07T14:54:55.714Z","level":"INFO","logger":"api_server","message":"User login successful","request_id":"req-12345","service":"user-api","user_id":"user-456","duration_ms":"145","ip_address":"192.168.1.100"}
```

**Benefits:**
- **Cloud-ready**: outputs JSON Lines format, ingestible by all major log platforms
- **Context preservation**: global context fields are automatically included in every log
- **Queryable fields**: structured data makes logs searchable and analyzable
- **Distributed tracing**: request IDs and correlation IDs flow through your logs
- **Performance metrics**: easily extract timing, error codes, and other numeric data

 
## Integrating Sinks Into Your Server or Service

XLog's sinks are small, composable objects you can attach to any `xlog::Logger`. Typical server integration patterns:

- **Create one logger per subsystem**: create a single logger for each subsystem (HTTP, DB, auth) and attach the sinks you need.

- **Attach multiple sinks**: mix console, file, and network sinks easily:

```cpp
#include <xlog/logger.hpp>
#include <xlog/sinks/file_sink.hpp>
#include <xlog/sinks/syslog_sink.hpp>
#include <xlog/sinks/udp_sink.hpp>

auto logger = std::make_shared<xlog::Logger>("http_server");
logger->add_sink(std::make_shared<xlog::FileSink>("/var/log/myserver.log"));
logger->add_sink(std::make_shared<xlog::SyslogSink>("myserver", LOG_PID, LOG_USER));
logger->add_sink(std::make_shared<xlog::UdpSink>("log-collector.example.local", 5140));
```

- **Per-environment configuration**: enable verbose sinks (console) only in development; enable syslog/remote sinks in production. Use your configuration system to create and attach sinks during startup.

- **Structured logs and aggregation**: for integration with aggregators or SIEMs, prefer structured outputs (JSON) or a TCP/HTTP sink. XLog includes experimental JSON sink and an ASIO-based `NetworkSink` for TCP; you can adapt or extend these to match your aggregator's ingest format.

- **CMake gating for platform-specific sinks**: the `SyslogSink` uses POSIX `syslog(3)` and is not portable to Windows. Consider adding a CMake option in your builds to enable/disable platform-specific sinks. For example:

```cmake
option(ENABLE_SYSLOG "Enable Syslog sink" ON)
if(ENABLE_SYSLOG)
    target_sources(xlog PRIVATE src/sinks/syslog_sink.cpp)
endif()
```

- **Best practices**:
    - Attach sinks at startup and reuse logger instances.
    - Keep network sinks asynchronous or non-blocking to avoid impacting request latency.
    - Rate-limit or sample high-volume logs before sending across the network.

## Contributing
1. Fork the repository
2. Create a branch for your feature/fix
3. Submit a pull request

We welcome improvements, bug fixes, and new sink implementations.

## LICENSE

MIT License. See [LICENSE](./LICENSE) for details.

## Summary

**XLog** is a lightweight, flexible logging solution for C++ projects. It makes logging structured, thread-safe, and easy to manage across complex applications. Whether you are debugging, profiling, or shipping a product, XLog helps keep your logs organized and your development process smooth
