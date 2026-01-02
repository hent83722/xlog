#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/structured_logger.hpp>
#include <Zyrnix/log_context.hpp>
#include <thread>
#include <chrono>
#include <iostream>

void process_payment(const std::string& payment_id) {
    auto logger = Zyrnix::StructuredLogger::create("payment_service", "payments.jsonl");
    
    auto ctx = Zyrnix::ScopedContext();
    ctx.set("payment_id", payment_id);
    ctx.set("service", "payment-processor");
    
    logger->info("Starting payment processing");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ctx.set("amount", "99.99");
    ctx.set("currency", "USD");
    
    logger->info("Validating payment method", {{"step", "validation"}});
    
    auto validate = [&]() {
        logger->info("Checking fraud rules", {{"check_type", "fraud_detection"}});
    };
    validate();
    
    logger->info("Payment completed successfully", {{"status", "success"}});
}

void handle_http_request(const std::string& request_id, const std::string& user_id) {
    auto logger = Zyrnix::StructuredLogger::create("http_server", "requests.jsonl");
    
    Zyrnix::ScopedContext request_ctx;
    request_ctx.set("request_id", request_id)
               .set("user_id", user_id)
               .set("endpoint", "/api/v1/orders");
    
    logger->info("Received HTTP request");
    
    {
        Zyrnix::ScopedContext db_ctx;
        db_ctx.set("operation", "database_query");
        
        logger->debug("Fetching user data from database", {{"table", "users"}});
    }
    
    logger->info("Request processing complete", {
        {"status_code", "200"},
        {"duration_ms", "145"}
    });
}

void worker_thread(int thread_id) {
    auto logger = Zyrnix::StructuredLogger::create("worker", "workers.jsonl");
    
    Zyrnix::ScopedContext ctx;
    ctx.set("thread_id", std::to_string(thread_id));
    ctx.set("worker_name", "worker-" + std::to_string(thread_id));
    
    for (int i = 0; i < 3; ++i) {
        logger->info("Processing task", {
            {"task_id", std::to_string(i)},
            {"iteration", std::to_string(i + 1)}
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void demonstrate_global_context() {
    Zyrnix::LogContext::set("app_version", "1.0.3");
    Zyrnix::LogContext::set("environment", "production");
    Zyrnix::LogContext::set("hostname", "server-01");
    
    auto logger = Zyrnix::StructuredLogger::create("app", "app.jsonl");
    
    logger->info("Application started");
    
    {
        Zyrnix::ScopedContext request_ctx;
        request_ctx.set("request_id", "req-789");
        
        logger->info("Processing request");
    }
    
    logger->info("Application running");
    
    Zyrnix::LogContext::clear();
}

int main() {
    std::cout << "=== Zyrnix Context & Scoped Attributes Demo ===\n\n";
    
    // Example 1: Payment processing with context
    std::cout << "1. Processing payment with scoped context...\n";
    process_payment("pay-12345");
    std::cout << "   Output written to payments.jsonl\n\n";
    
    // Example 2: HTTP request handling
    std::cout << "2. Handling HTTP request with correlation ID...\n";
    handle_http_request("req-abc123", "user-456");
    std::cout << "   Output written to requests.jsonl\n\n";
    
    // Example 3: Multi-threaded logging (context isolation)
    std::cout << "3. Multi-threaded workers (each has isolated context)...\n";
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(worker_thread, i);
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "   Output written to workers.jsonl\n\n";
    
    // Example 4: Global application context
    std::cout << "4. Global application-wide context...\n";
    demonstrate_global_context();
    std::cout << "   Output written to app.jsonl\n\n";
    
    std::cout << "=== Demo Complete ===\n";
    std::cout << "Check the .jsonl files to see structured output with context fields.\n";
    
    return 0;
}
