/**
 * XLog v1.1.1-beta.2 Features Demonstration
 * 
 * This example showcases the new features in v1.1.1-beta.2:
 * 
 * 1. Regex Filter Caching & Optimization
 *    - Pre-compiled static filters
 *    - Filter statistics (matches/misses count)
 *    - Case-insensitive matching option
 * 
 * 2. Health Check Improvements
 *    - Auto-registration of loggers
 *    - Aggregate health checks across all loggers
 *    - Configurable health thresholds per logger
 *    - "Last error message" in health check results
 * 
 * 3. Dynamic Log Level Enhancements
 *    - Per-sink level overrides
 *    - Level change history/audit trail
 *    - Temporary level changes with auto-revert
 *    - REST API helper for web-based log level control
 */

#include <xlog/xlog.hpp>
#include <xlog/log_filter.hpp>
#include <xlog/log_health.hpp>
#include <xlog/logger.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace xlog;
using namespace std::chrono_literals;

void demo_regex_filter_enhancements() {
    std::cout << "\n=== Regex Filter Caching & Optimization ===\n\n";
    
    auto logger = Logger::create_stdout_logger("regex_demo");
    
    // 1. Case-insensitive matching
    std::cout << "1. Case-insensitive regex filter:\n";
    RegexFilterOptions options;
    options.case_insensitive = true;
    options.track_stats = true;
    
    auto ci_filter = std::make_shared<RegexFilter>("error|warning", options);
    logger->add_filter(ci_filter);
    
    logger->info("This is an ERROR message");      // Should match (case-insensitive)
    logger->info("This has a Warning in it");      // Should match
    logger->info("This is just info");             // Should NOT match
    logger->info("CRITICAL ERROR DETECTED");       // Should match
    
    // Get filter statistics
    auto stats = ci_filter->get_stats();
    std::cout << "\nFilter statistics:\n";
    std::cout << "  Matches: " << stats.matches << "\n";
    std::cout << "  Misses: " << stats.misses << "\n";
    std::cout << "  Match rate: " << (stats.match_rate() * 100) << "%\n";
    
    // 2. Pre-compiled filter cache
    std::cout << "\n2. Pre-compiled filter cache:\n";
    
    auto& cache = RegexFilterCache::instance();
    
    // Pre-compile commonly used patterns
    cache.precompile("no_passwords", "(password|secret|token|api_key)", 
                    RegexFilterOptions{false, true, true});  // inverted, track stats
    cache.precompile("errors_only", "(ERROR|CRITICAL|FATAL)", 
                    RegexFilterOptions{true, false, true});  // case-insensitive
    
    // Use pre-compiled filters
    auto pwd_filter = cache.get_precompiled("no_passwords");
    if (pwd_filter) {
        logger->clear_filters();
        logger->add_filter(pwd_filter);
        
        logger->info("User logged in successfully");
        logger->info("Setting password to secret123");  // Should be filtered out
        logger->info("API call completed");
    }
    
    // Get or create cached filter (reuses existing if same pattern)
    auto filter1 = cache.get_or_create("\\d{4}-\\d{4}-\\d{4}-\\d{4}");  // Credit card pattern
    auto filter2 = cache.get_or_create("\\d{4}-\\d{4}-\\d{4}-\\d{4}");  // Same pattern - cached!
    
    std::cout << "Cache hits: " << cache.cache_hits() << "\n";
    std::cout << "Cache misses: " << cache.cache_misses() << "\n";
    std::cout << "Cache size: " << cache.cache_size() << "\n";
}

void demo_health_check_improvements() {
    std::cout << "\n=== Health Check Improvements ===\n\n";
    
    // 1. Enable auto-registration
    std::cout << "1. Auto-registration of loggers:\n";
    HealthRegistry::enable_auto_registration(true);
    
    // These loggers will auto-register with health registry
    auto api_logger = Logger::create_stdout_logger("api");
    auto db_logger = Logger::create_stdout_logger("database");
    auto cache_logger = Logger::create_stdout_logger("cache");
    
    // 2. Register health state change callback
    std::cout << "\n2. Health state change callback:\n";
    HealthRegistry::instance().register_state_change_callback(
        [](const std::string& name, HealthStatus old_status, 
           HealthStatus new_status, const HealthCheckResult& result) {
            std::cout << "Health state changed for '" << name << "': ";
            std::cout << (old_status == HealthStatus::Healthy ? "Healthy" : 
                         old_status == HealthStatus::Degraded ? "Degraded" : "Unhealthy");
            std::cout << " -> ";
            std::cout << (new_status == HealthStatus::Healthy ? "Healthy" : 
                         new_status == HealthStatus::Degraded ? "Degraded" : "Unhealthy");
            std::cout << "\n";
        });
    
    // 3. Per-logger health configuration
    std::cout << "\n3. Per-logger health configuration:\n";
    HealthCheckConfig strict_config;
    strict_config.max_drop_rate_healthy = 0.001;  // 0.1% for critical API logger
    strict_config.max_latency_us_healthy = 5000;  // 5ms max latency
    
    HealthRegistry::instance().set_logger_config("api", strict_config);
    std::cout << "Set strict health config for 'api' logger\n";
    
    // 4. Record errors for debugging
    std::cout << "\n4. Last error tracking:\n";
    HealthRegistry::instance().record_error("database", 
        "Connection timeout after 30s to primary replica");
    
    // 5. Aggregate health check
    std::cout << "\n5. Aggregate health check:\n";
    auto aggregate = handle_aggregate_health_check();
    std::cout << aggregate.to_string();
    
    std::cout << "\nJSON output:\n" << aggregate.to_json() << "\n";
    
    // Cleanup
    HealthRegistry::enable_auto_registration(false);
}

void demo_dynamic_log_level_enhancements() {
    std::cout << "\n=== Dynamic Log Level Enhancements ===\n\n";
    
    auto logger = Logger::create_stdout_logger("dynamic_demo");
    logger->set_level(LogLevel::Info);
    
    // 1. Level change with reason
    std::cout << "1. Level change with reason (audit trail):\n";
    logger->set_level_dynamic(LogLevel::Debug, "Debugging production issue #12345");
    logger->set_level_dynamic(LogLevel::Trace, "Need more detail for RCA");
    logger->set_level_dynamic(LogLevel::Info, "Issue resolved, reverting to normal");
    
    // Get level change history
    auto history = logger->get_level_history();
    std::cout << "\nLevel change history:\n";
    for (const auto& entry : history) {
        auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
        std::cout << "  " << std::ctime(&time);
        std::cout << "    Changed from level " << static_cast<int>(entry.old_level)
                  << " to " << static_cast<int>(entry.new_level);
        if (!entry.reason.empty()) {
            std::cout << " - Reason: " << entry.reason;
        }
        std::cout << "\n";
    }
    
    // 2. Temporary level change with auto-revert
    std::cout << "\n2. Temporary level change (5 second duration):\n";
    logger->set_level_temporary(LogLevel::Debug, 5s, "Temporary debugging");
    
    std::cout << "Current level: " << static_cast<int>(logger->get_level()) << " (Debug)\n";
    std::cout << "Has temporary level: " << (logger->has_temporary_level() ? "yes" : "no") << "\n";
    std::cout << "Remaining duration: " << logger->remaining_temporary_duration().count() << "s\n";
    
    // Log some debug messages while in temporary mode
    logger->debug("This debug message will appear");
    logger->trace("This trace message won't appear (still below Debug)");
    
    std::cout << "\nWaiting 3 seconds...\n";
    std::this_thread::sleep_for(3s);
    std::cout << "Remaining duration: " << logger->remaining_temporary_duration().count() << "s\n";
    
    // Cancel early
    logger->cancel_temporary_level();
    std::cout << "Cancelled temporary level\n";
    std::cout << "Current level: " << static_cast<int>(logger->get_level()) << " (Info)\n";
    
    // 3. Per-sink level overrides
    std::cout << "\n3. Per-sink level overrides:\n";
    // Note: In a real scenario, you'd add multiple sinks
    // logger->add_sink(file_sink);  // index 0: stdout, index 1: file
    // logger->set_sink_level(0, LogLevel::Info);   // Console: Info and above
    // logger->set_sink_level(1, LogLevel::Debug);  // File: Debug and above
    std::cout << "Per-sink levels allow different sinks to receive different log levels\n";
    std::cout << "Example: Console=Info, File=Debug, Syslog=Error\n";
    
    // 4. REST API helper
    std::cout << "\n4. REST API helper for web-based control:\n";
    
    // Simulate REST API request
    auto response = handle_level_change_request(logger, "debug", 
        "Changed via admin API", 0);
    std::cout << "API Response:\n" << response.to_json() << "\n";
    
    // Temporary change via API
    response = handle_level_change_request(logger, "trace", 
        "Temporary debug session", 60);  // 60 seconds
    std::cout << "\nTemporary change response:\n" << response.to_json() << "\n";
    
    // Invalid level
    response = handle_level_change_request(logger, "invalid_level", "", 0);
    std::cout << "\nInvalid level response:\n" << response.to_json() << "\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         XLog v1.1.1-beta.2 Features Demonstration          ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    demo_regex_filter_enhancements();
    demo_health_check_improvements();
    demo_dynamic_log_level_enhancements();
    
    std::cout << "\n=== Demo Complete ===\n";
    return 0;
}
