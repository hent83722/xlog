#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "logger.hpp"
#include "log_macros.hpp"
#include "log_filter.hpp"
#include "log_context.hpp"
#include "sinks/stdout_sink.hpp"

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

void demo_basic_logging() {
    print_section("Demo 1: Basic Logging with XLog");
    
    // Create logger with stdout sink
    auto logger = xlog::Logger::create_stdout_logger("demo");
    
    // Basic logging
    logger->trace("This is a trace message");
    logger->debug("This is a debug message");
    logger->info("This is an info message");
    logger->warn("This is a warning message");
    logger->error("This is an error message");
    logger->critical("This is a critical message");
    
    std::cout << "\n  ✓ All log levels demonstrated\n";
}

void demo_compile_time_macros() {
    print_section("Demo 2: Compile-Time Filtering Macros");
    
    auto logger = xlog::Logger::create_stdout_logger("compile_time");
    
    std::cout << "  Using XLOG_* macros (compile-time filterable):\n\n";
    
    // These macros are eliminated at compile time in release builds
    XLOG_TRACE(logger, "Trace: Eliminated in release build");
    XLOG_DEBUG(logger, "Debug: Eliminated in release build");
    XLOG_INFO(logger, "Info: Always included");
    XLOG_WARN(logger, "Warn: Always included");
    XLOG_ERROR(logger, "Error: Always included");
    XLOG_CRITICAL(logger, "Critical: Always included");
    
    std::cout << "\n  ✓ In Release builds (-DNDEBUG), TRACE and DEBUG are eliminated!\n";
}

void demo_conditional_macros() {
    print_section("Demo 3: Conditional Logging Macros");
    
    auto logger = xlog::Logger::create_stdout_logger("conditional");
    
    std::cout << "  Only logs when condition is true:\n\n";
    
    for (int i = 0; i < 10; i++) {
        bool is_even = (i % 2 == 0);
        XLOG_INFO_IF(logger, is_even, "Even number processed");
        
        bool has_error = (i == 5);
        XLOG_ERROR_IF(logger, has_error, "Error at iteration 5!");
    }
    
    std::cout << "\n  ✓ Conditional macros prevent message construction when false\n";
}

void demo_runtime_level_filter() {
    print_section("Demo 4: Runtime Level Filtering");
    
    auto logger = xlog::Logger::create_stdout_logger("runtime");
    
    std::cout << "  Without filter - all logs appear:\n\n";
    
    logger->debug("Debug message");
    logger->info("Info message");
    logger->warn("Warning message");
    logger->error("Error message");
    
    std::cout << "\n  Now adding filter: Only WARN and above:\n\n";
    
    // Add level filter
    auto level_filter = std::make_shared<xlog::LevelFilter>(xlog::LogLevel::Warn);
    logger->add_filter(level_filter);
    
    logger->debug("Debug message - FILTERED OUT");
    logger->info("Info message - FILTERED OUT");
    logger->warn("Warning message - APPEARS");
    logger->error("Error message - APPEARS");
    
    std::cout << "\n  ✓ Runtime filter dynamically controls log output\n";
}

void demo_field_based_filter() {
    print_section("Demo 5: Field-Based Filtering");
    
    auto logger = xlog::Logger::create_stdout_logger("field_filter");
    
    std::cout << "  Adding filter: Only logs with user_type=premium:\n\n";
    
    // Filter for premium users only
    auto field_filter = std::make_shared<xlog::FieldFilter>("user_type", "premium");
    logger->add_filter(field_filter);
    
    // Standard user - filtered out
    {
        xlog::ScopedContext ctx({{"user_type", "standard"}});
        logger->info("Standard user activity - FILTERED");
    }
    
    // Premium user - appears
    {
        xlog::ScopedContext ctx({{"user_type", "premium"}});
        logger->info("Premium user activity - APPEARS");
    }
    
    std::cout << "\n  ✓ Field-based filtering works with scoped context\n";
}

void demo_composite_filter() {
    print_section("Demo 6: Composite Filters (AND Logic)");
    
    auto logger = xlog::Logger::create_stdout_logger("composite");
    
    std::cout << "  Filter: (Level >= INFO) AND (has 'urgent' field):\n\n";
    
    // Create composite filter
    auto composite = std::make_shared<xlog::CompositeFilter>(
        xlog::CompositeFilter::Mode::AND
    );
    composite->add_filter(std::make_shared<xlog::LevelFilter>(xlog::LogLevel::Info));
    composite->add_filter(std::make_shared<xlog::FieldFilter>("urgent", "true"));
    logger->add_filter(composite);
    
    // Not urgent - filtered
    logger->info("Regular info message - FILTERED (no urgent field)");
    
    // Urgent - appears
    {
        xlog::ScopedContext ctx({{"urgent", "true"}});
        logger->info("Urgent info message - APPEARS");
        logger->error("Urgent error message - APPEARS");
    }
    
    std::cout << "\n  ✓ Composite filters combine multiple conditions\n";
}

void demo_lambda_filter() {
    print_section("Demo 7: Lambda Filters (Custom Logic)");
    
    auto logger = xlog::Logger::create_stdout_logger("lambda");
    
    std::cout << "  Filter: Errors OR messages with priority=critical:\n\n";
    
    // Custom lambda filter
    auto lambda_filter = std::make_shared<xlog::LambdaFilter>(
        [](const xlog::LogRecord& record) {
            return record.level >= xlog::LogLevel::Error ||
                   (record.has_field("priority") && record.get_field("priority") == "critical");
        }
    );
    logger->add_filter(lambda_filter);
    
    logger->info("Regular info - FILTERED");
    
    {
        xlog::ScopedContext ctx({{"priority", "critical"}});
        logger->info("Critical priority info - APPEARS");
    }
    
    logger->error("Error message - APPEARS");
    
    std::cout << "\n  ✓ Lambda filters enable complex custom logic\n";
}

void demo_scoped_context() {
    print_section("Demo 8: Scoped Context (Automatic Field Propagation)");
    
    auto logger = xlog::Logger::create_stdout_logger("context");
    
    std::cout << "  Using scoped context to add fields automatically:\n\n";
    
    {
        xlog::ScopedContext request_ctx({
            {"request_id", "req-12345"},
            {"user", "alice"},
            {"endpoint", "POST /api/tasks"}
        });
        
        logger->info("Processing API request");
        logger->info("Validating permissions");
        logger->info("Request completed successfully");
    }
    
    std::cout << "\n  ✓ Scoped context automatically adds fields to all logs\n";
}

void demo_performance() {
    print_section("Demo 9: Performance Comparison");
    
    auto logger = xlog::Logger::create_stdout_logger("perf");
    const int iterations = 100000;
    
    // No filtering
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        logger->debug("Test message");
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto no_filter_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // With filtering (filter out debug)
    logger->add_filter(std::make_shared<xlog::LevelFilter>(xlog::LogLevel::Info));
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        logger->debug("Test message");
    }
    end = std::chrono::high_resolution_clock::now();
    auto filtered_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "  Iterations: " << iterations << "\n";
    std::cout << "  Without filtering: " << no_filter_time << " μs\n";
    std::cout << "  With filtering:    " << filtered_time << " μs\n";
    std::cout << "  Time saved:        " << (no_filter_time - filtered_time) << " μs\n";
    std::cout << "\n  ✓ Filtering prevents expensive log operations!\n";
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║        XLog v1.0.4 Feature Demonstration                   ║\n";
    std::cout << "║        Conditional Logging & Zero-Cost Abstractions        ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    demo_basic_logging();
    demo_compile_time_macros();
    demo_conditional_macros();
    demo_runtime_level_filter();
    demo_field_based_filter();
    demo_composite_filter();
    demo_lambda_filter();
    demo_scoped_context();
    demo_performance();
    
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ✓ All XLog v1.0.4 features demonstrated successfully!    ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    return 0;
}
