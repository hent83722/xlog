# XLog - Lightweight C++ Logging Library

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**XLog** is a modern, lightweight, and high-performance logging library for C++17, inspired by popular logging libraries like `spdlog`. It provides thread-safe logging, multiple sinks, log levels, and flexible formatting, making it ideal for both small projects and large-scale applications.

---

## Features

-  **Header-only library** (compile as static library optionally)
-  **Multiple log levels:** Trace, Debug, Info, Warn, Error, Critical
-  **Multiple sinks:** Console, File, Rotating File, Daily File, Custom Sinks
-  **Thread-safe and asynchronous logging**
-  **Stream-style logging syntax** (`logger << xlog::Info << "Message" << xlog::endl;`)
-  **Flexible formatting:** Timestamps, colors, structured messages, JSON support (experimental)
-  **Minimal dependencies:** Uses standard C++17 and optional third-party fmt library for formatting

---

## Project Structure

```t
.
├── include/xlog # Public headers
├── src # Implementation files
├── examples # Example usage
├── tests # Unit tests
├── benchmarks # Performance benchmarks
├── cmake # Build scripts and CMake helpers
└── docs # Documentation and diagrams
```
---

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/hent83722/xlog.git
cd xlog
```
### 2. Build the static library

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```
This will generate a static library libxlog.a in the build folder.


### 3. Add XLog to your project
In your CMake project:

```Cmake
add_subdirectory(path/to/xlog)
target_link_libraries(your_project PRIVATE xlog)
target_include_directories(your_project PRIVATE path/to/xlog/include)
```


## Usage

### 1. Basic Logging

```cpp
#include <xlog/xlog.hpp>

int main() {
    auto logger = xlog::Logger::create_stdout_logger("my_logger");

    // Log messages with levels
    logger->log(xlog::LogLevel::Info, "This is an info message!");
    logger->log(xlog::LogLevel::Warn, "This is a warning message!");
    logger->log(xlog::LogLevel::Error, "This is an error message!");

    return 0;
}
```

### 2. Stream-style Logging
```cpp
*logger << xlog::Info << "Stream-style logging works too!" << xlog::endl;
```

- Combines the simplicity of std::cout with log levels.

- Automatically prepends timestamps and level info.

### 3. Multiple Sinks

```cpp
logger->add_sink(std::make_shared<xlog::StdoutSink>());
logger->add_sink(std::make_shared<xlog::FileSink>("app.log"));
```

- Sends logs to both console and file.

- Can add rotating file sinks, daily logs, or custom sinks.

### 4. Asynchronous Logging

```cpp
auto async_logger = xlog::Logger::create_async("async_logger");
async_logger->log(xlog::LogLevel::Info, "This message is logged asynchronously!");
```

- Logging operations won't block your main program.

- Great for performance-critical or multi-threaded applications.

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

## Examples


See the examples folder for detailed use cases:

- basic_logging.cpp – Simple console logging.

- async_logging.cpp – Asynchronous logging in multi-threaded environments.

- file_vs_stdout.cpp – Logging to multiple sinks.

- rotating_logs.cpp – Rotating file logging for large projects.

## Contributing

1. Fork the repository
2. Create a branch for your feature/fix
3. Submit a pull request

We welcome improvements, bug fixes, and new sink implementations.

## LICENSE

MIT License. See [LICENSE](./LICENSE) for details.

## Summary

**XLog** is a lightweight, flexible logging solution for C++ projects. It makes logging structured, thread-safe, and easy to manage across complex applications. Whether you are debugging, profiling, or shipping a product, XLog helps keep your logs organized and your development process smooth

