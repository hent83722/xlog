/**
 * @file rate_limiting_example.cpp
 * @brief Demonstrates rate limiting and sampling features
 * 
 * This example shows:
 * 1. Token bucket rate limiting (messages per second)
 * 2. Sampling (log every Nth message)
 * 3. Combined rate limiting and sampling
 * 4. Statistics tracking
 */

#include <Zyrnix/logger.hpp>
#include <Zyrnix/rate_limiter.hpp>
#include <Zyrnix/sinks/stdout_sink.hpp>
#include <iostream>
#include <thread>
#include <chrono>

void example_rate_limiting() {
    std::cout << "\n=== Rate Limiting Example ===\n";
    
    // Create rate limiter: max 10 messages per second, burst capacity of 20
    Zyrnix::RateLimiter limiter(10, 20);
    
    std::cout << "Attempting to log 100 messages rapidly...\n";
    
    int logged = 0;
    int dropped = 0;
    
    for (int i = 0; i < 100; ++i) {
        if (limiter.try_log()) {
            logged++;
            std::cout << "Message " << i << " logged\n";
        } else {
            dropped++;
        }
    }
    
    std::cout << "\nResults:\n";
    std::cout << "  Logged: " << logged << "\n";
    std::cout << "  Dropped: " << dropped << "\n";
    std::cout << "  Rate limiter dropped: " << limiter.dropped_count() << "\n";
}

void example_sampling() {
    std::cout << "\n=== Sampling Example ===\n";
    
    // Create sampling limiter: log every 10th message
    Zyrnix::SamplingLimiter sampler(10);
    
    std::cout << "Logging 100 messages with 1-in-10 sampling...\n";
    
    for (int i = 0; i < 100; ++i) {
        if (sampler.should_log()) {
            std::cout << "Message " << i << " logged (sampled)\n";
        }
    }
    
    std::cout << "\nResults:\n";
    std::cout << "  Total messages: " << sampler.total_count() << "\n";
    std::cout << "  Dropped (sampled out): " << sampler.dropped_count() << "\n";
}

void example_combined_limiting() {
    std::cout << "\n=== Combined Rate Limiting + Sampling Example ===\n";
    
    // Combine rate limiting (100 msg/s) and sampling (1 in 5)
    Zyrnix::CombinedLimiter limiter(100, 150, 5);
    
    std::cout << "Simulating high-throughput logging...\n";
    
    // Simulate 1 second of high-frequency logging
    auto start = std::chrono::steady_clock::now();
    int logged = 0;
    
    for (int i = 0; i < 1000; ++i) {
        if (limiter.should_log()) {
            logged++;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500)); // 2000 msg/s
    }
    
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
    ).count();
    
    auto stats = limiter.get_stats();
    
    std::cout << "\nResults (elapsed: " << elapsed << " ms):\n";
    std::cout << "  Total messages: " << stats.total_messages << "\n";
    std::cout << "  Logged: " << stats.logged_messages << "\n";
    std::cout << "  Sampling drops: " << stats.sampling_drops << "\n";
    std::cout << "  Rate limit drops: " << stats.rate_limited_drops << "\n";
    std::cout << "  Effective rate: " << (stats.logged_messages * 1000.0 / elapsed) << " msg/s\n";
}

void example_production_scenario() {
    std::cout << "\n=== Production Scenario: Preventing Log Flooding ===\n";
    
    auto logger = std::make_shared<Zyrnix::Logger>("app");
    logger->add_sink(std::make_shared<Zyrnix::StdoutSink>());
    
    // Rate limiter to prevent disk exhaustion during incidents
    Zyrnix::RateLimiter rate_limiter(50, 100); // 50 msg/s, burst 100
    
    std::cout << "Simulating an error storm (1000 errors rapidly)...\n";
    
    int logged = 0;
    for (int i = 0; i < 1000; ++i) {
        if (rate_limiter.try_log()) {
            logger->error("Database connection failed (attempt " + std::to_string(i) + ")");
            logged++;
        }
    }
    
    std::cout << "\nProtected the system by rate limiting:\n";
    std::cout << "  Would have logged: 1000 messages\n";
    std::cout << "  Actually logged: " << logged << " messages\n";
    std::cout << "  Prevented: " << (1000 - logged) << " messages from flooding disk\n";
    std::cout << "  Dropped by rate limiter: " << rate_limiter.dropped_count() << "\n";
}

int main() {
    std::cout << "Zyrnix Rate Limiting & Sampling Examples\n";
    std::cout << "========================================\n";
    
    example_rate_limiting();
    example_sampling();
    example_combined_limiting();
    example_production_scenario();
    
    std::cout << "\n=== Key Takeaways ===\n";
    std::cout << "1. Rate limiting prevents log flooding during incidents\n";
    std::cout << "2. Sampling reduces log volume in high-throughput scenarios\n";
    std::cout << "3. Combined approach provides flexible control\n";
    std::cout << "4. Token bucket algorithm allows bursts while maintaining limits\n";
    std::cout << "5. Statistics help monitor dropped message counts\n";
    
    return 0;
}
