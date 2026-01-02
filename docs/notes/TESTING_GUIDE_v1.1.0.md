# Zyrnix v1.1.0 Testing Guide

## Quick Start Testing

### Build the Comprehensive Test

```bash
cd /home/henri/Zyrnix
g++ -std=c++17 examples/comprehensive_test.cpp -I include -L build -lZyrnix -lpthread -o test_v1.1.0
./test_v1.1.0
```

### Expected Output

The test will run 8 real-world scenarios demonstrating all v1.1.0 features.

---

## Scenario Descriptions

### Scenario 1: Application Startup
**Tests:** Basic logging functionality  
**Expected:** Clean startup messages showing logging works

### Scenario 2: Error Storm with Rate Limiting
**Tests:** Token bucket rate limiting  
**Expected:** 
- 100 errors attempted
- Only ~15-20 logged (rate limit: 5 msg/sec, burst: 10)
- 80-85 dropped
- Statistics showing dropped count

### Scenario 3: Sampling High-Frequency Debug Logs
**Tests:** Sampling limiter (1 in 20)  
**Expected:**
- 200 transactions processed
- Only 10 debug logs written
- 95% volume reduction

### Scenario 4: Metrics & Observability
**Tests:** Metrics collection, Prometheus export, JSON export  
**Expected:**
- Performance metrics (messages/sec, latency)
- Health metrics (drops, errors, queue depth)
- Prometheus format output
- JSON format output

### Scenario 5: Multi-Sink Production Logging
**Tests:** Multiple sinks simultaneously  
**Expected:**
- Messages appear on console
- Messages written to production.log file
- File created in current directory

### Scenario 6: Combined Rate Limiting + Sampling
**Tests:** Combined limiter (rate + sampling)  
**Expected:**
- 1000 messages attempted
- Heavy reduction from both rate and sampling
- Statistics showing both types of drops

### Scenario 7: Real-World E-Commerce System
**Tests:** Full system simulation with multiple loggers  
**Expected:**
- App logger tracks orders
- Payment logger uses rate limiting
- Inventory logger uses sampling
- Metrics collected for all loggers
- Realistic production behavior

### Scenario 8: Health Monitoring Dashboard
**Tests:** Global metrics registry, health checks  
**Expected:**
- Health status for all loggers
- Per-logger statistics
- Health indicators (✓ or ✗)

---

## Individual Feature Tests

### Test 1: Rate Limiting

```bash
cd /home/henri/Zyrnix
g++ -std=c++17 examples/rate_limiting_example.cpp -I include -L build -lZyrnix -lpthread -o test_rate_limiting
./test_rate_limiting
```

**What to verify:**
- Token bucket allows initial burst
- Rate is maintained over time
- Dropped count increases correctly
- Statistics are accurate

### Test 2: Compression

```bash
g++ -std=c++17 examples/compression_example.cpp -I include -L build -lZyrnix -lpthread -lz -lzstd -o test_compression
./test_compression
```

**What to verify:**
- Log files are created
- Rotated files are compressed (.gz or .zst extension)
- Compression statistics show space savings
- Original files are deleted after compression

**Check compressed files:**
```bash
ls -lh logs/
file logs/*.gz logs/*.zst
```

### Test 3: Cloud Sinks

```bash
export AWS_ACCESS_KEY_ID="your-key"
export AWS_SECRET_ACCESS_KEY="your-secret"
export APPINSIGHTS_INSTRUMENTATIONKEY="your-key"

g++ -std=c++17 examples/cloud_sinks_example.cpp -I include -L build -lZyrnix -lpthread -lcurl -o test_cloud
./test_cloud
```

**What to verify:**
- Batching behavior (messages queued, sent in batches)
- Retry logic on failure
- Statistics show sent/failed/dropped counts
- Non-blocking async operation

### Test 4: Metrics

```bash
g++ -std=c++17 examples/metrics_example.cpp -I include -L build -lZyrnix -lpthread -o test_metrics
./test_metrics
```

**What to verify:**
- Metrics are collected during logging
- Prometheus format is valid
- JSON format is valid
- Per-logger and per-sink metrics work
- Global registry tracks all loggers

---

## Real-Life Scenario Simulations

### Scenario A: Web Server Under Load

```cpp
#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/rate_limiter.hpp>
#include <Zyrnix/log_metrics.hpp>

void simulate_web_server() {
    auto access_log = Zyrnix::Logger::create_stdout_logger("access");
    auto error_log = Zyrnix::Logger::create_stdout_logger("error");
    
    Zyrnix::RateLimiter error_limiter(10, 20);
    
    auto& registry = Zyrnix::MetricsRegistry::instance();
    auto metrics = registry.get_logger_metrics("access");
    
    for (int i = 0; i < 10000; ++i) {
        metrics->record_message_logged();
        access_log->info("GET /api/users HTTP/1.1 200 123ms");
        
        if (i % 100 == 0) {
            if (error_limiter.try_log()) {
                error_log->error("Database connection timeout");
            }
        }
    }
    
    auto snapshot = metrics->get_snapshot();
    std::cout << "Requests: " << snapshot.messages_logged << "\n";
    std::cout << "Rate: " << snapshot.messages_per_second << " req/s\n";
}
```

**Simulates:** High-traffic web server with error handling

### Scenario B: Microservices with Distributed Tracing

```cpp
#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/log_metrics.hpp>

void process_request(const std::string& request_id) {
    auto api_logger = Zyrnix::Logger::create_stdout_logger("api-gateway");
    auto auth_logger = Zyrnix::Logger::create_stdout_logger("auth-service");
    auto db_logger = Zyrnix::Logger::create_stdout_logger("db-service");
    
    api_logger->info("Request " + request_id + " received");
    auth_logger->info("Request " + request_id + " authenticated");
    db_logger->debug("Request " + request_id + " query executed");
    api_logger->info("Request " + request_id + " completed");
}

void simulate_microservices() {
    for (int i = 0; i < 100; ++i) {
        std::string req_id = "req-" + std::to_string(1000 + i);
        process_request(req_id);
    }
}
```

**Simulates:** Microservices architecture with request tracking

### Scenario C: Batch Processing with Progress Logging

```cpp
#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/rate_limiter.hpp>

void simulate_batch_processing() {
    auto logger = Zyrnix::Logger::create_stdout_logger("batch-processor");
    Zyrnix::SamplingLimiter sampler(1000);
    
    logger->info("Starting batch processing of 1,000,000 records");
    
    for (int i = 1; i <= 1000000; ++i) {
        if (sampler.should_log()) {
            logger->info("Processed " + std::to_string(i) + " records");
        }
        
        if (i % 100000 == 0) {
            logger->info("Progress: " + std::to_string(i / 10000) + "%");
        }
    }
    
    logger->info("Batch processing completed");
    std::cout << "Total debug logs: " << (sampler.total_count() / 1000) << "\n";
}
```

**Simulates:** Large batch job with progress tracking

### Scenario D: Real-Time Monitoring Dashboard

```cpp
#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/log_metrics.hpp>
#include <thread>
#include <chrono>

void monitoring_dashboard() {
    auto& registry = Zyrnix::MetricsRegistry::instance();
    
    auto logger = Zyrnix::Logger::create_stdout_logger("app");
    auto metrics = registry.get_logger_metrics("app");
    
    std::thread worker([&]() {
        for (int i = 0; i < 10000; ++i) {
            metrics->record_message_logged();
            logger->info("Processing event " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    for (int i = 0; i < 5; ++i) {
        auto snapshot = metrics->get_snapshot();
        std::cout << "\n=== Dashboard Update ===\n";
        std::cout << "Timestamp: " << i << "s\n";
        std::cout << "Messages: " << snapshot.messages_logged << "\n";
        std::cout << "Rate: " << snapshot.messages_per_second << " msg/s\n";
        std::cout << "Latency: " << snapshot.avg_log_latency_us << " µs\n";
        std::cout << "Queue: " << snapshot.current_queue_depth << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    worker.join();
}
```

**Simulates:** Real-time monitoring with live metrics updates

---

## Performance Benchmarking

### Benchmark 1: Rate Limiter Overhead

```cpp
#include <Zyrnix/rate_limiter.hpp>
#include <chrono>
#include <iostream>

void benchmark_rate_limiter() {
    Zyrnix::RateLimiter limiter(10000, 20000);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; ++i) {
        limiter.try_log();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    std::cout << "Rate limiter overhead: " << (duration / 1000000) << " ns per call\n";
}
```

**Expected:** 50-100 nanoseconds per call

### Benchmark 2: Metrics Overhead

```cpp
#include <Zyrnix/log_metrics.hpp>
#include <chrono>

void benchmark_metrics() {
    Zyrnix::LogMetrics metrics;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; ++i) {
        metrics.record_message_logged();
        metrics.record_log_duration(10);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    std::cout << "Metrics overhead: " << (duration / 1000000) << " ns per call\n";
}
```

**Expected:** 10-20 nanoseconds per call

---

## Integration Testing

### Test with Existing Zyrnix Features

```cpp
#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/structured_logger.hpp>
#include <Zyrnix/log_context.hpp>
#include <Zyrnix/rate_limiter.hpp>
#include <Zyrnix/log_metrics.hpp>

void integration_test() {
    auto slog = Zyrnix::StructuredLogger::create("api", "api.jsonl");
    
    Zyrnix::ScopedContext ctx;
    ctx.set("request_id", "req-12345").set("user_id", "user-456");
    
    Zyrnix::RateLimiter limiter(100, 200);
    
    auto& registry = Zyrnix::MetricsRegistry::instance();
    auto metrics = registry.get_logger_metrics("api");
    
    for (int i = 0; i < 1000; ++i) {
        if (limiter.try_log()) {
            metrics->record_message_logged();
            slog->info("API request processed", {
                {"endpoint", "/api/users"},
                {"method", "GET"},
                {"status", "200"}
            });
        }
    }
    
    auto snapshot = metrics->get_snapshot();
    std::cout << "Integration test:\n";
    std::cout << "  Logged: " << snapshot.messages_logged << "\n";
    std::cout << "  Dropped: " << limiter.dropped_count() << "\n";
    std::cout << "  Rate: " << snapshot.messages_per_second << " msg/s\n";
}
```

**Verifies:** New features work with existing v1.0.4 features

---

## Continuous Monitoring Test

### Setup Prometheus Endpoint

```cpp
#include <Zyrnix/log_metrics.hpp>
#include <thread>
#include <chrono>
#include <fstream>

void prometheus_exporter() {
    auto& registry = Zyrnix::MetricsRegistry::instance();
    
    while (true) {
        std::string metrics = registry.export_all_prometheus("Zyrnix");
        
        std::ofstream out("metrics.prom");
        out << metrics;
        out.close();
        
        std::cout << "Metrics exported to metrics.prom\n";
        
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }
}
```

**Usage:** Run in background, configure Prometheus to scrape `metrics.prom`

---

## Troubleshooting

### Issue: Rate limiter not limiting

**Check:**
- Ensure rate is set > 0
- Verify `try_log()` return value is checked
- Check if time is advancing (not in tight loop without delays)

### Issue: Metrics showing zero

**Check:**
- Metrics are recorded during logging
- Using correct logger name with registry
- `record_message_logged()` is called

### Issue: Sampling not working

**Check:**
- Sample rate > 1 (1 = log all)
- `should_log()` return value is checked
- Counter is incrementing

---

## Success Criteria

### All Tests Pass If:

1. ✓ Rate limiting reduces message volume significantly
2. ✓ Sampling provides predictable reduction (1/N)
3. ✓ Metrics show non-zero values for logged messages
4. ✓ Prometheus export format is valid
5. ✓ JSON export format is valid
6. ✓ Multi-sink logging writes to all destinations
7. ✓ Combined limiters provide multiplicative reduction
8. ✓ Health monitoring shows all loggers
9. ✓ No crashes or memory leaks
10. ✓ Performance overhead < 100ns per operation

---

## Next Steps

After successful testing:

1. Deploy to staging environment
2. Monitor metrics in production
3. Set up Prometheus/Grafana dashboards
4. Configure alerts for drop rates > 1%
5. Tune rate limits based on actual traffic
6. Optimize compression levels for your data
7. Configure cloud sinks for centralized logging
8. Document your production configuration

---

**Happy Testing! 🚀**
