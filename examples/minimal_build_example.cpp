/**
 * Example: Conditional Compilation for Minimal Binary Size
 * 
 * Demonstrates how to use XLOG feature flags to reduce binary size
 * by disabling unnecessary features at compile time.
 * 
 * Compile with different flags to see size differences:
 * 
 * Full build:
 *   g++ -std=c++17 minimal_build_example.cpp -lZyrnix -o full
 *   
 * Minimal build:
 *   g++ -std=c++17 -DXLOG_MINIMAL minimal_build_example.cpp -lZyrnix -o minimal
 *   
 * Custom build (disable specific features):
 *   g++ -std=c++17 -DXLOG_NO_ASYNC -DXLOG_NO_JSON minimal_build_example.cpp -lZyrnix -o custom
 * 
 * Or use CMake:
 *   cmake -DXLOG_MINIMAL=ON ..
 *   cmake -DXLOG_ENABLE_ASYNC=OFF -DXLOG_ENABLE_JSON=OFF ..
 */

#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/logger.hpp>
#include <Zyrnix/sinks/stdout_sink.hpp>
#include <Zyrnix/sinks/file_sink.hpp>
#include <iostream>

// Feature detection examples
void show_features() {
    std::cout << "=== Zyrnix Feature Configuration ===" << std::endl;
    std::cout << std::endl;
    
#if XLOG_HAS_ASYNC
    std::cout << "✓ Async logging:      ENABLED" << std::endl;
#else
    std::cout << "✗ Async logging:      DISABLED (save ~15-20KB)" << std::endl;
#endif

#if XLOG_HAS_JSON
    std::cout << "✓ JSON logging:       ENABLED" << std::endl;
#else
    std::cout << "✗ JSON logging:       DISABLED (save ~10-15KB)" << std::endl;
#endif

#if XLOG_HAS_NETWORK
    std::cout << "✓ Network sinks:      ENABLED" << std::endl;
#else
    std::cout << "✗ Network sinks:      DISABLED (save ~8-12KB)" << std::endl;
#endif

#if XLOG_HAS_COLORS
    std::cout << "✓ Color output:       ENABLED" << std::endl;
#else
    std::cout << "✗ Color output:       DISABLED (save ~2-3KB)" << std::endl;
#endif

#if XLOG_HAS_FILE_ROTATION
    std::cout << "✓ File rotation:      ENABLED" << std::endl;
#else
    std::cout << "✗ File rotation:      DISABLED (save ~5-8KB)" << std::endl;
#endif

#if XLOG_HAS_CONTEXT
    std::cout << "✓ Log context:        ENABLED" << std::endl;
#else
    std::cout << "✗ Log context:        DISABLED (save ~3-5KB)" << std::endl;
#endif

#if XLOG_HAS_FILTERS
    std::cout << "✓ Log filters:        ENABLED" << std::endl;
#else
    std::cout << "✗ Log filters:        DISABLED (save ~2-4KB)" << std::endl;
#endif

    std::cout << std::endl;
    std::cout << "Total binary size reduction: ~50-70KB with XLOG_MINIMAL" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Zyrnix Minimal Build Example ===" << std::endl;
    std::cout << std::endl;
    
    show_features();
    
    // Basic logging always works
    std::cout << "=== Basic Logging (Always Available) ===" << std::endl;
    auto logger = Zyrnix::Logger::create_stdout_logger("minimal");
    logger->info("Basic logging works in all builds");
    logger->warn("Warning message");
    logger->error("Error message");
    std::cout << std::endl;
    
    // Async logging (only if enabled)
#if XLOG_HAS_ASYNC
    std::cout << "=== Async Logging ===" << std::endl;
    auto async_logger = Zyrnix::Logger::create_async("async");
    async_logger->info("Async logging is available");
    std::cout << std::endl;
#else
    std::cout << "=== Async Logging ===" << std::endl;
    std::cout << "Async logging is disabled in this build" << std::endl;
    std::cout << std::endl;
#endif
    
    // JSON logging (only if enabled)
#if XLOG_HAS_JSON
    std::cout << "=== Structured/JSON Logging ===" << std::endl;
    std::cout << "JSON logging is available (not shown in this basic example)" << std::endl;
    std::cout << std::endl;
#else
    std::cout << "=== Structured/JSON Logging ===" << std::endl;
    std::cout << "JSON logging is disabled in this build" << std::endl;
    std::cout << std::endl;
#endif
    
    // Context logging (only if enabled)
#if XLOG_HAS_CONTEXT
    std::cout << "=== Context Logging ===" << std::endl;
    std::cout << "Log context (MDC/NDC) is available" << std::endl;
    std::cout << std::endl;
#else
    std::cout << "=== Context Logging ===" << std::endl;
    std::cout << "Log context is disabled in this build" << std::endl;
    std::cout << std::endl;
#endif
    
    // Usage recommendations
    std::cout << "=== Build Recommendations ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Embedded/IoT devices:" << std::endl;
    std::cout << "  Use -DXLOG_MINIMAL for smallest binary" << std::endl;
    std::cout << std::endl;
    std::cout << "Desktop applications:" << std::endl;
    std::cout << "  Disable only unused features (e.g., -DXLOG_NO_NETWORK)" << std::endl;
    std::cout << std::endl;
    std::cout << "Servers/Cloud:" << std::endl;
    std::cout << "  Use full build with all features enabled" << std::endl;
    std::cout << std::endl;
    
    // CMake examples
    std::cout << "=== CMake Configuration Examples ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Minimal build:" << std::endl;
    std::cout << "  cmake -DXLOG_MINIMAL=ON .." << std::endl;
    std::cout << std::endl;
    std::cout << "Custom build:" << std::endl;
    std::cout << "  cmake -DXLOG_ENABLE_ASYNC=OFF \\" << std::endl;
    std::cout << "        -DXLOG_ENABLE_JSON=OFF \\" << std::endl;
    std::cout << "        -DXLOG_ENABLE_NETWORK=OFF .." << std::endl;
    std::cout << std::endl;
    std::cout << "Per-target build:" << std::endl;
    std::cout << "  target_compile_definitions(my_app PRIVATE XLOG_NO_ASYNC XLOG_NO_JSON)" << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Example completed ===" << std::endl;
    
    return 0;
}
