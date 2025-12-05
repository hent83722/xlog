# XLog - Lightweight C++ Logging Library

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
 
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
## Contributing
1. Fork the repository
2. Create a branch for your feature/fix
3. Submit a pull request

We welcome improvements, bug fixes, and new sink implementations.

## LICENSE

MIT License. See [LICENSE](./LICENSE) for details.

## Summary

**XLog** is a lightweight, flexible logging solution for C++ projects. It makes logging structured, thread-safe, and easy to manage across complex applications. Whether you are debugging, profiling, or shipping a product, XLog helps keep your logs organized and your development process smooth
