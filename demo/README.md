# XLog v1.0.4 Demonstration Application

A complete, working demonstration of **XLog v1.0.4** conditional logging and zero-cost abstractions features.

## Quick Start

```bash
cd /home/henri/xlog-demo-app
cd build
./xlog_demo
```

## Features Demonstrated

### 1. **Basic Logging**
- All 6 log levels: TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
- Simple logger creation with stdout sink

### 2. **Compile-Time Log Elimination**
- TRACE and DEBUG logs automatically removed in release builds
- Zero runtime overhead for disabled logs
- Uses `XLOG_TRACE`, `XLOG_DEBUG`, `XLOG_INFO`, etc. macros

### 3. **Conditional Logging Macros**
- `XLOG_*_IF` macros prevent message construction when conditions are false
- Only evaluates log arguments when conditions are met
- Perfect for performance-critical paths

### 4. **Runtime Level Filtering**
- Dynamically filter logs by minimum level
- No code changes needed to adjust log verbosity
- `LevelFilter` stops logs below threshold

### 5. **Field-Based Filtering**
- Filter based on context field values
- Works with scoped context
- `FieldFilter` checks specific field values

### 6. **Composite Filters**
- Combine multiple filters with AND/OR logic
- Build complex filtering rules
- `CompositeFilter` with flexible modes

### 7. **Lambda Filters**
- Custom filter logic with C++ lambdas
- Complete control over filtering decisions
- `LambdaFilter` accepts any predicate function

### 8. **Scoped Context**
- Automatic field propagation with `ScopedContext`
- Thread-local context support
- Fields appear in all logs within scope

### 9. **Performance Testing**
- Real benchmarks showing filtering benefits
- 100,000 iterations comparing filtered vs. unfiltered
- Demonstrates massive time savings (~1.7 seconds saved!)

## Project Structure

```
xlog-demo-app/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
└── src/
    └── simple_demo.cpp      # Complete demonstration (9 demos)
```

## Building

### Prerequisites
- C++17 compiler (GCC 7+, Clang 5+)
- CMake 3.16+
- XLog library v1.0.4 (located at `../xlog`)

### Build Steps

```bash
# Make sure XLog is built first
cd /home/henri/xlog
mkdir -p build && cd build
cmake .. && make

# Build the demo
cd /home/henri/xlog-demo-app
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# For release build (eliminates TRACE/DEBUG at compile time)
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DNDEBUG" .. && make
```

## Running

```bash
cd /home/henri/xlog-demo-app/build
./xlog_demo
```

## What You'll See

The demo runs 9 comprehensive demonstrations:

1. **Basic Logging** - All 6 log levels demonstrated
2. **Compile-Time Macros** - XLOG_* macros with compile-time elimination
3. **Conditional Macros** - XLOG_*_IF macros prevent unnecessary work
4. **Runtime Level Filter** - Dynamic filtering by log level
5. **Field-Based Filter** - Filter logs by context field values
6. **Composite Filter** - Combine filters with AND/OR logic
7. **Lambda Filter** - Custom filtering with predicates
8. **Scoped Context** - Automatic field propagation
9. **Performance Comparison** - Benchmark showing 1.7 second savings for 100K logs!

## Key Code Examples from Demo

### Basic Logging
```cpp
auto logger = xlog::Logger::create_stdout_logger("myapp");
logger->info("Application started");
logger->warn("Low memory warning");
logger->error("Connection failed");
```

### Compile-Time Filtering
```cpp
// In debug builds: all logs appear
// In release builds (-DNDEBUG): TRACE and DEBUG eliminated
XLOG_TRACE(logger, "Detailed trace");        // Gone in release!
XLOG_DEBUG(logger, "Debug info");            // Gone in release!
XLOG_INFO(logger, "Important info");         // Always present
```

### Conditional Logging
```cpp
// Only logs when condition is true - prevents message construction
XLOG_INFO_IF(logger, is_premium_user, "Premium feature used");
XLOG_ERROR_IF(logger, has_error, "Error occurred");
```

### Runtime Level Filtering
```cpp
// Add filter - now only WARN and above will appear
auto filter = std::make_shared<xlog::LevelFilter>(xlog::LogLevel::Warn);
logger->add_filter(filter);

logger->debug("Hidden");  // Filtered out
logger->warn("Shown");    // Appears
```

### Field-Based Filtering
```cpp
// Only log messages with user_type=premium
auto filter = std::make_shared<xlog::FieldFilter>("user_type", "premium");
logger->add_filter(filter);

xlog::ScopedContext ctx({{"user_type", "premium"}});
logger->info("This appears");  // Has the field
```

### Lambda Filters
```cpp
// Custom logic: errors OR urgent messages
auto filter = std::make_shared<xlog::LambdaFilter>(
    [](const xlog::LogRecord& record) {
        return record.level >= xlog::LogLevel::Error ||
               record.has_field("urgent");
    }
);
logger->add_filter(filter);
```

### Scoped Context
```cpp
{
    xlog::ScopedContext ctx({
        {"request_id", "req-123"},
        {"user", "alice"}
    });
    
    logger->info("Processing");  // Includes request_id and user
    logger->info("Complete");    // Also includes both fields
}
// Fields automatically removed after scope
```

## Performance Results

From the demo's benchmark (100,000 log operations):

- **Without filtering**: 1,769,755 μs (~1.77 seconds)
- **With filtering**: 34,812 μs (~0.03 seconds)
- **Time saved**: 1,734,943 μs (~1.73 seconds) - **98% faster!**

This shows that filtering prevents:
- String formatting operations
- Sink write operations
- Lock acquisition
- Memory allocation

## Learning Points

This demo shows how XLog v1.0.4 features work in practice:

✅ **Zero-cost compile-time elimination** - TRACE/DEBUG removed in release builds  
✅ **Runtime filtering** - Control log output without code changes  
✅ **Conditional logging** - Prevent expensive operations when not needed  
✅ **Context propagation** - Automatic field addition to related logs  
✅ **Flexible architecture** - Mix and match filters for complex rules  
✅ **Thread-safe** - All features work correctly in multi-threaded apps  
✅ **Measurable benefits** - 98% performance improvement demonstrated!  

## Experiment Ideas

Try modifying `src/simple_demo.cpp` to:

1. **Test compile-time elimination**: Rebuild with `-DNDEBUG` and see TRACE/DEBUG disappear
2. **Create custom filters**: Write your own lambda filter with complex logic
3. **Add more context fields**: Extend scoped context with additional metadata
4. **Benchmark your use case**: Modify the performance test for your scenario
5. **Combine filters**: Try OR mode in CompositeFilter
6. **Add file sinks**: Route filtered logs to different files

## Integration into Your Project

```cpp
// In your CMakeLists.txt
set(XLOG_DIR "/path/to/xlog")
include_directories(${XLOG_DIR}/include ${XLOG_DIR}/include/xlog)
target_link_libraries(your_app ${XLOG_DIR}/build/libxlog.a pthread)

// In your code
#include "logger.hpp"
#include "log_macros.hpp"
#include "log_filter.hpp"

auto logger = xlog::Logger::create_stdout_logger("your_app");
XLOG_INFO(logger, "Application started");
```

## Next Steps

- Review the source code in `src/simple_demo.cpp` - it's well-commented
- Check the main XLog README at `/home/henri/xlog/README.md`
- Read release notes at `/home/henri/xlog/docs/notes/v1.0.4.md`
- Explore other examples in `/home/henri/xlog/examples/`
