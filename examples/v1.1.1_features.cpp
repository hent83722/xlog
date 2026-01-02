/**
 * v1.1.1 New Features Example
 * 
 * This example demonstrates the four new features added in v1.1.1-beta.1:
 * 1. Regex-based log filtering
 * 2. Dynamic log level changes
 * 3. Health check API
 * 4. Compression auto-tune
 */

#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/log_filter.hpp>
#include <Zyrnix/log_health.hpp>
#include <Zyrnix/sinks/compressed_file_sink.hpp>
#include <Zyrnix/log_metrics.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace Zyrnix;

// ============================================================================
// 1. Regex-Based Filtering Example
// ============================================================================

void demo_regex_filtering() {
    std::cout << "\n=== Regex Filtering Demo ===\n";
    
    auto logger = Logger::create_stdout_logger("regex_demo");
    
    // Create a regex filter that only logs messages containing "ERROR" or "CRITICAL"
    auto error_filter = std::make_shared<RegexFilter>("(ERROR|CRITICAL)");
    logger->add_filter(error_filter);
    
    std::cout << "Logging with regex filter (only ERROR|CRITICAL messages):\n";
    logger->info("Normal info message");           // Filtered out
    logger->warn("Warning message");               // Filtered out
    logger->error("ERROR: Database connection failed");  // Logged
    logger->critical("CRITICAL: System failure");  // Logged
    
    // Clear filter and add inverted regex (log everything EXCEPT debug messages)
    logger->clear_filters();
    auto no_debug_filter = std::make_shared<RegexFilter>("DEBUG", true);  // invert=true
    logger->add_filter(no_debug_filter);
    
    std::cout << "\nLogging with inverted regex (exclude DEBUG):\n";
    logger->debug("DEBUG: Detailed information");  // Filtered out
    logger->info("INFO: Application started");     // Logged
    logger->warn("WARN: Low memory");              // Logged
}

// ============================================================================
// 2. Dynamic Log Level Changes Example
// ============================================================================

void demo_dynamic_log_levels() {
    std::cout << "\n=== Dynamic Log Level Changes Demo ===\n";
    
    auto logger = Logger::create_stdout_logger("dynamic_level");
    
    // Register callback to be notified of level changes
    logger->register_level_change_callback([](LogLevel old_level, LogLevel new_level) {
        std::cout << "🔔 Log level changed from " 
                  << static_cast<int>(old_level) << " to " 
                  << static_cast<int>(new_level) << "\n";
    });
    
    logger->set_level(LogLevel::Info);
    logger->info("Application started");
    logger->debug("This debug message won't appear");
    
    // Simulate runtime configuration change (e.g., from config file or API)
    std::cout << "\nEnabling debug logging at runtime...\n";
    logger->set_level_dynamic(LogLevel::Debug);  // Thread-safe, triggers callback
    
    logger->debug("Now debug messages appear!");
    logger->info("Still logging info");
    
    // Change back to Info
    std::cout << "\nDisabling debug logging...\n";
    logger->set_level_dynamic(LogLevel::Info);
    logger->debug("This debug message won't appear again");
    logger->info("Back to info level");
}

// ============================================================================
// 3. Health Check API Example
// ============================================================================

void demo_health_checks() {
    std::cout << "\n=== Health Check Demo ===\n";
    
    // Create logger with metrics
    auto logger = Logger::create_stdout_logger("health_demo");
    LogMetrics metrics;
    
    // Register logger with health registry
    HealthRegistry::instance().register_logger("health_demo", logger);
    
    // Simulate some logging activity
    for (int i = 0; i < 100; ++i) {
        logger->info("Message " + std::to_string(i));
        metrics.record_message_logged();
        
        if (i % 20 == 0) {
            metrics.record_error();  // Simulate some errors
        }
    }
    
    // Perform health check
    HealthChecker checker;
    auto result = checker.check_metrics(metrics);
    
    std::cout << "\nHealth Check Result:\n";
    std::cout << result.to_string() << "\n";
    
    std::cout << "\nHealth Check JSON:\n";
    std::cout << result.to_json() << "\n";
    
    // Check overall system health
    std::cout << "\nOverall System Health: ";
    switch (HealthRegistry::instance().get_overall_status()) {
        case HealthStatus::Healthy: std::cout << "✅ HEALTHY\n"; break;
        case HealthStatus::Degraded: std::cout << "⚠️  DEGRADED\n"; break;
        case HealthStatus::Unhealthy: std::cout << "❌ UNHEALTHY\n"; break;
    }
}

// ============================================================================
// 4. Compression Auto-Tune Example
// ============================================================================

void demo_compression_autotune() {
    std::cout << "\n=== Compression Auto-Tune Demo ===\n";
    
    // Create compressed sink with auto-tune enabled
    CompressionOptions options;
    options.type = CompressionType::Gzip;
    options.level = 6;  // Start with default level
    options.compress_on_rotate = true;
    options.auto_tune = true;  // Enable auto-tune
    
    auto sink = std::make_shared<CompressedFileSink>(
        "autotune_test.log",
        1024 * 1024,  // 1 MB rotation
        3,             // Keep 3 files
        options
    );
    
    auto logger = std::make_shared<Logger>("autotune_demo");
    logger->add_sink(sink);
    
    std::cout << "Initial compression level: " << sink->get_current_compression_level() << "\n";
    
    // Simulate heavy logging to trigger rotation and auto-tune
    std::cout << "Generating logs to trigger rotation and auto-tune...\n";
    for (int i = 0; i < 5000; ++i) {
        logger->info("This is a test message with some content to compress. "
                    "The auto-tune feature will adjust compression level based on "
                    "performance metrics like compression ratio and speed. "
                    "Message number: " + std::to_string(i));
        
        if (i % 1000 == 0) {
            std::cout << "  Compression level: " << sink->get_current_compression_level() << "\n";
        }
    }
    
    // Display compression statistics
    auto stats = sink->get_compression_stats();
    std::cout << "\nCompression Statistics:\n";
    std::cout << "  Files compressed: " << stats.files_compressed << "\n";
    std::cout << "  Original size: " << stats.original_bytes << " bytes\n";
    std::cout << "  Compressed size: " << stats.compressed_bytes << " bytes\n";
    std::cout << "  Compression ratio: " << stats.compression_ratio << "x\n";
    std::cout << "  Space saved: " 
              << (100.0 - (100.0 * stats.compressed_bytes / stats.original_bytes)) 
              << "%\n";
    std::cout << "  Final compression level: " << sink->get_current_compression_level() << "\n";
}

// ============================================================================
// Combined Usage Example
// ============================================================================

void demo_combined_features() {
    std::cout << "\n=== Combined Features Demo ===\n";
    std::cout << "Using all v1.1.1 features together\n";
    
    // Create logger with compressed sink and auto-tune
    CompressionOptions options;
    options.auto_tune = true;
    auto sink = std::make_shared<CompressedFileSink>(
        "combined_demo.log", 512 * 1024, 5, options
    );
    
    auto logger = std::make_shared<Logger>("combined");
    logger->add_sink(sink);
    
    // Add regex filter for sensitive data
    auto filter = std::make_shared<RegexFilter>("(password|secret|token)", true);  // invert
    logger->add_filter(filter);
    
    // Register level change callback
    logger->register_level_change_callback([](LogLevel old_level, LogLevel new_level) {
        std::cout << "📊 Adjusted log level based on runtime conditions\n";
    });
    
    // Register for health monitoring
    HealthRegistry::instance().register_logger("combined", logger);
    
    // Simulate application with dynamic behavior
    logger->info("Application initialized with all v1.1.1 features enabled");
    
    // Dynamic level adjustment based on condition
    bool debug_mode = true;
    if (debug_mode) {
        logger->set_level_dynamic(LogLevel::Debug);
        logger->debug("Debug mode activated");
    }
    
    // Log some messages (sensitive data will be filtered)
    logger->info("User logged in: john@example.com");
    logger->warn("Invalid password attempt");  // Filtered out by regex!
    logger->error("Database connection timeout");
    
    // Check health
    auto health_result = HealthRegistry::instance().check_logger("combined");
    std::cout << "Health Status: ";
    if (HealthChecker::is_healthy(health_result)) {
        std::cout << "✅ Healthy\n";
    } else {
        std::cout << "⚠️  Issues detected\n";
    }
    
    std::cout << "Compression level auto-adjusted to: " 
              << sink->get_current_compression_level() << "\n";
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     Zyrnix v1.1.1-beta.1 New Features Demonstration       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    
    try {
        demo_regex_filtering();
        demo_dynamic_log_levels();
        demo_health_checks();
        demo_compression_autotune();
        demo_combined_features();
        
        std::cout << "\n✅ All demos completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
