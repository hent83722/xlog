#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>

#include "logger.hpp"
#include "log_macros.hpp"
#include "log_filter.hpp"
#include "log_context.hpp"
#include "sinks/stdout_sink.hpp"

#include "task_manager.hpp"
#include "user_service.hpp"
#include "api_handler.hpp"

using namespace taskapp;

// Helper to format strings
template<typename... Args>
std::string fmt(const char* format_str, Args&&... args) {
    std::ostringstream oss;
    ((oss << args << " "), ...);
    return oss.str();
}

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

void demo_basic_operations(std::shared_ptr<ApiHandler> api) {
    print_section("Demo 1: Basic Task Operations");
    
    // Register and login users
    api->handle_login("alice", "password123");
    api->handle_login("bob", "password456");
    
    // Create various priority tasks
    api->handle_create_task("alice", "Fix critical bug in production", "critical");
    api->handle_create_task("alice", "Update documentation", "low");
    api->handle_create_task("bob", "Code review PR #123", "medium");
    api->handle_create_task("bob", "Deploy to staging", "high");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void demo_conditional_logging(std::shared_ptr<xlog::Logger> logger) {
    print_section("Demo 2: Conditional Logging (XLOG_*_IF macros)");
    
    // These logs only execute if condition is true
    for (int i = 0; i < 10; i++) {
        // Only logs premium users (even numbers in this demo)
        bool is_premium = (i % 2 == 0);
        XLOG_INFO_IF(logger, is_premium, "Processing premium user request: {}", i);
        
        // Only logs errors
        bool has_error = (i % 5 == 0);
        XLOG_ERROR_IF(logger, has_error, "Error encountered in request: {}", i);
    }
    
    std::cout << "  ✓ Conditional logs only executed when conditions were true\n";
}

void demo_runtime_filters(std::shared_ptr<xlog::Logger> logger) {
    print_section("Demo 3: Runtime Filtering");
    
    std::cout << "  Setting filter: Only WARN and above\n\n";
    
    // Add level filter - only logs WARN and above
    auto level_filter = std::make_shared<xlog::LevelFilter>(xlog::LogLevel::Warn);
    logger->add_filter(level_filter);
    
    // These will be filtered out
    XLOG_DEBUG(logger, "This debug log will be filtered out");
    XLOG_INFO(logger, "This info log will be filtered out");
    
    // These will appear
    XLOG_WARN(logger, "This warning will appear");
    XLOG_ERROR(logger, "This error will appear");
    
    std::cout << "\n  ✓ Debug and Info logs were filtered, Warn and Error appeared\n";
    
    // Clear filters for next demos
    logger->clear_filters();
}

void demo_field_based_filtering(std::shared_ptr<xlog::Logger> logger,
                                std::shared_ptr<ApiHandler> api) {
    print_section("Demo 4: Field-Based Filtering (Context Fields)");
    
    std::cout << "  Setting filter: Only logs with 'user_type' = 'premium'\n\n";
    
    // Only log messages that have user_type = premium
    auto field_filter = std::make_shared<xlog::FieldFilter>("user_type", "premium");
    logger->add_filter(field_filter);
    
    // Login different user types
    api->handle_login("alice", "pass");  // standard user
    
    xlog::LogContext::add_field("user_type", "standard");
    XLOG_INFO(logger, "Standard user logged in - THIS WILL BE FILTERED");
    
    xlog::LogContext::add_field("user_type", "premium");
    XLOG_INFO(logger, "Premium user logged in - THIS WILL APPEAR");
    
    std::cout << "\n  ✓ Only premium user logs appeared\n";
    
    logger->clear_filters();
    xlog::LogContext::clear();
}

void demo_composite_filters(std::shared_ptr<xlog::Logger> logger) {
    print_section("Demo 5: Composite Filters (AND/OR Logic)");
    
    std::cout << "  Setting composite filter: (Level >= INFO) AND (has field 'urgent')\n\n";
    
    // Create composite filter with AND logic
    auto composite = std::make_shared<xlog::CompositeFilter>(
        xlog::CompositeFilter::Mode::AND
    );
    composite->add_filter(std::make_shared<xlog::LevelFilter>(xlog::LogLevel::Info));
    composite->add_filter(std::make_shared<xlog::FieldFilter>("urgent", "true"));
    
    logger->add_filter(composite);
    
    // This won't pass (no urgent field)
    XLOG_INFO(logger, "Regular info message - FILTERED");
    
    // This will pass (has urgent field and level >= INFO)
    xlog::ScopedContext ctx({{"urgent", "true"}});
    XLOG_INFO(logger, "Urgent info message - APPEARS");
    XLOG_ERROR(logger, "Urgent error message - APPEARS");
    
    std::cout << "\n  ✓ Only logs with INFO+ level AND urgent=true appeared\n";
    
    logger->clear_filters();
}

void demo_lambda_filters(std::shared_ptr<xlog::Logger> logger) {
    print_section("Demo 6: Lambda Filters (Custom Logic)");
    
    std::cout << "  Setting lambda filter: Only errors OR messages with 'critical' priority\n\n";
    
    // Custom filter logic
    auto lambda_filter = std::make_shared<xlog::LambdaFilter>(
        [](const xlog::LogRecord& record) {
            return record.level >= xlog::LogLevel::Error ||
                   (record.has_field("priority") && record.get_field("priority") == "critical");
        }
    );
    logger->add_filter(lambda_filter);
    
    XLOG_INFO(logger, "Regular info - FILTERED");
    
    xlog::LogContext::add_field("priority", "critical");
    XLOG_INFO(logger, "Critical priority info - APPEARS");
    xlog::LogContext::clear();
    
    XLOG_ERROR(logger, "Error message - APPEARS");
    
    std::cout << "\n  ✓ Custom lambda filter applied successfully\n";
    
    logger->clear_filters();
}

void demo_compile_time_filtering() {
    print_section("Demo 7: Compile-Time Filtering");
    
    std::cout << "  In DEBUG builds:\n";
    std::cout << "    - XLOG_TRACE and XLOG_DEBUG are compiled in\n";
    std::cout << "  In RELEASE builds (-DNDEBUG):\n";
    std::cout << "    - XLOG_TRACE and XLOG_DEBUG are eliminated (zero cost)\n\n";
    
    auto logger = xlog::Logger::create("compile_time_demo");
    
    XLOG_TRACE(logger, "This trace log exists in debug, eliminated in release");
    XLOG_DEBUG(logger, "This debug log exists in debug, eliminated in release");
    XLOG_INFO(logger, "This info log always exists");
    
    std::cout << "\n  ✓ Compile-time filtering demonstrated\n";
    std::cout << "  ℹ️  Rebuild with -DNDEBUG to see TRACE/DEBUG eliminated\n";
}

void demo_performance_comparison(std::shared_ptr<xlog::Logger> logger) {
    print_section("Demo 8: Performance Comparison");
    
    const int iterations = 100000;
    
    // Benchmark: No filtering
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        XLOG_DEBUG(logger, "Test message {}", i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto no_filter_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Benchmark: With level filter (filtering out debug)
    logger->add_filter(std::make_shared<xlog::LevelFilter>(xlog::LogLevel::Info));
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        XLOG_DEBUG(logger, "Test message {}", i);
    }
    end = std::chrono::high_resolution_clock::now();
    auto filtered_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    logger->clear_filters();
    
    std::cout << "  Iterations: " << iterations << "\n";
    std::cout << "  No filtering: " << no_filter_time << " μs\n";
    std::cout << "  With filtering: " << filtered_time << " μs (filtered out)\n";
    std::cout << "  Time saved: " << (no_filter_time - filtered_time) << " μs\n";
    std::cout << "\n  ✓ Filtering prevents expensive operations!\n";
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║        XLog v1.0.4 Demonstration Application               ║\n";
    std::cout << "║        Task Manager with Advanced Logging                  ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    // Create loggers with different sinks
    auto app_logger = xlog::Logger::create("taskapp");
    auto api_logger = xlog::Logger::create("api");
    
    // Add global context (appears in all logs)
    xlog::LogContext::add_field("app", "taskmanager");
    xlog::LogContext::add_field("version", "1.0.0");
    xlog::LogContext::add_field("environment", "demo");
    
    // Create services
    auto user_service = std::make_shared<UserService>(app_logger);
    auto task_manager = std::make_shared<TaskManager>(app_logger);
    auto api_handler = std::make_shared<ApiHandler>(api_logger, task_manager, user_service);
    
    // Register demo users
    user_service->register_user("alice", "alice@example.com", UserRole::User, false);
    user_service->register_user("bob", "bob@example.com", UserRole::Admin, true);
    user_service->register_user("charlie", "charlie@example.com", UserRole::User, true);
    
    // Run all demos
    demo_basic_operations(api_handler);
    demo_conditional_logging(api_logger);
    demo_runtime_filters(api_logger);
    demo_field_based_filtering(api_logger, api_handler);
    demo_composite_filters(api_logger);
    demo_lambda_filters(api_logger);
    demo_compile_time_filtering();
    demo_performance_comparison(api_logger);
    
    // Final statistics
    print_section("Application Summary");
    task_manager->print_statistics();
    
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ✓ All XLog v1.0.4 features demonstrated successfully!    ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    return 0;
}
