/**
 * @file metrics_example.cpp
 * @brief Demonstrates metrics and observability features
 * 
 * This example shows:
 * 1. Collecting logger metrics (messages/sec, latency, etc.)
 * 2. Per-sink statistics
 * 3. Prometheus export format
 * 4. JSON metrics export
 * 5. Global metrics registry
 */

#include <Zyrnix/logger.hpp>
#include <Zyrnix/log_metrics.hpp>
#include <Zyrnix/sinks/file_sink.hpp>
#include <Zyrnix/sinks/stdout_sink.hpp>
#include <iostream>
#include <thread>
#include <chrono>

void example_basic_metrics() {
    std::cout << "\n=== Basic Metrics Example ===\n";
    
    auto metrics = std::make_shared<Zyrnix::LogMetrics>();
    
    // Simulate logging activity
    for (int i = 0; i < 1000; ++i) {
        metrics->record_message_logged();
        
        if (i % 100 == 0) {
            metrics->record_message_dropped();
        }
        
        if (i % 50 == 0) {
            metrics->record_message_filtered();
        }
        
        // Simulate log latency
        metrics->record_log_duration(10 + (i % 20)); // 10-30 microseconds
    }
    
    // Simulate some flushes
    for (int i = 0; i < 5; ++i) {
        metrics->record_flush();
        metrics->record_flush_duration(500 + i * 100);
    }
    
    // Get snapshot
    auto snapshot = metrics->get_snapshot();
    
    std::cout << "Metrics Snapshot:\n";
    std::cout << "  Messages logged: " << snapshot.messages_logged << "\n";
    std::cout << "  Messages dropped: " << snapshot.messages_dropped << "\n";
    std::cout << "  Messages filtered: " << snapshot.messages_filtered << "\n";
    std::cout << "  Flushes: " << snapshot.flushes << "\n";
    std::cout << "  Messages/second: " << snapshot.messages_per_second << "\n";
    std::cout << "  Avg log latency: " << snapshot.avg_log_latency_us << " µs\n";
    std::cout << "  Max log latency: " << snapshot.max_log_latency_us << " µs\n";
    std::cout << "  Avg flush latency: " << snapshot.avg_flush_latency_us << " µs\n";
    std::cout << "  Max flush latency: " << snapshot.max_flush_latency_us << " µs\n";
}

void example_prometheus_export() {
    std::cout << "\n=== Prometheus Export Example ===\n";
    
    auto metrics = std::make_shared<Zyrnix::LogMetrics>();
    
    // Generate some activity
    for (int i = 0; i < 5000; ++i) {
        metrics->record_message_logged();
        metrics->record_log_duration(15);
    }
    
    metrics->record_message_dropped();
    metrics->record_message_dropped();
    metrics->update_queue_depth(125);
    
    // Export in Prometheus format
    std::string prom_output = metrics->export_prometheus("myapp_log");
    
    std::cout << "Prometheus Metrics Format:\n";
    std::cout << "----------------------------\n";
    std::cout << prom_output;
    
    std::cout << "\nThese metrics can be scraped by Prometheus and visualized in Grafana\n";
}

void example_json_export() {
    std::cout << "\n=== JSON Export Example ===\n";
    
    auto metrics = std::make_shared<Zyrnix::LogMetrics>();
    
    // Simulate activity
    for (int i = 0; i < 2000; ++i) {
        metrics->record_message_logged();
    }
    metrics->record_message_dropped();
    metrics->record_error();
    
    std::string json = metrics->export_json();
    
    std::cout << "JSON Metrics:\n";
    std::cout << json << "\n\n";
    
    std::cout << "This format is ideal for:\n";
    std::cout << "  - REST API endpoints\n";
    std::cout << "  - Monitoring dashboards\n";
    std::cout << "  - Log aggregators (ELK, Splunk)\n";
}

void example_sink_metrics() {
    std::cout << "\n=== Per-Sink Metrics Example ===\n";
    
    auto file_metrics = std::make_shared<Zyrnix::SinkMetrics>("file_sink");
    auto stdout_metrics = std::make_shared<Zyrnix::SinkMetrics>("stdout_sink");
    
    // Simulate sink activity
    for (int i = 0; i < 1000; ++i) {
        file_metrics->record_write(256);  // 256 bytes per write
        file_metrics->record_write_duration(50);
        
        stdout_metrics->record_write(128); // 128 bytes
        stdout_metrics->record_write_duration(10);
    }
    
    file_metrics->record_flush();
    file_metrics->record_flush();
    stdout_metrics->record_flush();
    
    std::cout << "File Sink Statistics:\n";
    std::cout << "  Writes: " << file_metrics->get_writes() << "\n";
    std::cout << "  Bytes written: " << file_metrics->get_bytes_written() << "\n";
    std::cout << "  Flushes: " << file_metrics->get_flushes() << "\n";
    std::cout << "  Avg write latency: " << file_metrics->get_average_write_latency_us() << " µs\n\n";
    
    std::cout << "Stdout Sink Statistics:\n";
    std::cout << "  Writes: " << stdout_metrics->get_writes() << "\n";
    std::cout << "  Bytes written: " << stdout_metrics->get_bytes_written() << "\n";
    std::cout << "  Avg write latency: " << stdout_metrics->get_average_write_latency_us() << " µs\n";
}

void example_global_registry() {
    std::cout << "\n=== Global Metrics Registry Example ===\n";
    
    auto& registry = Zyrnix::MetricsRegistry::instance();
    
    // Create metrics for multiple loggers
    auto app_metrics = registry.get_logger_metrics("app");
    auto api_metrics = registry.get_logger_metrics("api");
    auto db_metrics = registry.get_logger_metrics("database");
    
    // Simulate activity
    for (int i = 0; i < 1000; ++i) app_metrics->record_message_logged();
    for (int i = 0; i < 500; ++i) api_metrics->record_message_logged();
    for (int i = 0; i < 2000; ++i) db_metrics->record_message_logged();
    
    app_metrics->record_message_dropped();
    api_metrics->record_error();
    
    // Get all snapshots
    auto snapshots = registry.get_all_logger_snapshots();
    
    std::cout << "All Logger Metrics:\n";
    for (const auto& [name, snapshot] : snapshots) {
        std::cout << "\nLogger: " << name << "\n";
        std::cout << "  Messages: " << snapshot.messages_logged << "\n";
        std::cout << "  Dropped: " << snapshot.messages_dropped << "\n";
        std::cout << "  Errors: " << snapshot.errors << "\n";
        std::cout << "  Rate: " << snapshot.messages_per_second << " msg/s\n";
    }
    
    // Export all metrics
    std::cout << "\n=== Combined Prometheus Export ===\n";
    std::string all_metrics = registry.export_all_prometheus();
    std::cout << all_metrics.substr(0, 500) << "...\n"; // Show first 500 chars
}

void example_monitoring_endpoint() {
    std::cout << "\n=== HTTP Monitoring Endpoint Pattern ===\n";
    
    auto& registry = Zyrnix::MetricsRegistry::instance();
    
    // Simulate some application activity
    auto app = registry.get_logger_metrics("app");
    for (int i = 0; i < 10000; ++i) {
        app->record_message_logged();
        app->record_log_duration(12);
    }
    
    std::cout << "Example HTTP endpoint implementation:\n";
    std::cout << "------------------------------------\n";
    std::cout << "GET /metrics (Prometheus format):\n\n";
    
    std::string prometheus = registry.export_all_prometheus("Zyrnix");
    std::cout << prometheus << "\n";
    
    std::cout << "------------------------------------\n";
    std::cout << "GET /metrics/json:\n\n";
    
    std::string json = registry.export_all_json();
    std::cout << json << "\n";
    
    std::cout << "\nIntegrate with your HTTP server:\n";
    std::cout << "  - Expose /metrics endpoint for Prometheus scraping\n";
    std::cout << "  - Use JSON endpoint for custom dashboards\n";
    std::cout << "  - Monitor queue depth to detect backpressure\n";
    std::cout << "  - Alert on high drop rates or error counts\n";
}

void example_production_monitoring() {
    std::cout << "\n=== Production Monitoring Example ===\n";
    
    auto& registry = Zyrnix::MetricsRegistry::instance();
    auto metrics = registry.get_logger_metrics("production");
    
    // Simulate production workload
    std::cout << "Simulating production workload...\n";
    
    for (int i = 0; i < 100000; ++i) {
        metrics->record_message_logged();
        metrics->record_log_duration(8 + (i % 10));
        
        if (i % 1000 == 0) {
            metrics->update_queue_depth(i / 100);
        }
    }
    
    metrics->record_message_dropped();
    metrics->record_message_dropped();
    metrics->record_message_dropped();
    
    auto snapshot = metrics->get_snapshot();
    
    std::cout << "\nProduction Metrics Dashboard:\n";
    std::cout << "=============================\n";
    std::cout << "Throughput:  " << snapshot.messages_per_second << " msg/s\n";
    std::cout << "Total Logs:  " << snapshot.messages_logged << "\n";
    std::cout << "Dropped:     " << snapshot.messages_dropped << " (" 
              << (100.0 * snapshot.messages_dropped / snapshot.messages_logged) << "%)\n";
    std::cout << "Latency (avg): " << snapshot.avg_log_latency_us << " µs\n";
    std::cout << "Latency (p99): " << snapshot.max_log_latency_us << " µs\n";
    std::cout << "Queue Depth: " << snapshot.current_queue_depth << " / " 
              << snapshot.max_queue_depth << "\n";
    
    std::cout << "\nHealth Check:\n";
    bool healthy = snapshot.messages_dropped < 10 && 
                   snapshot.avg_log_latency_us < 100 &&
                   snapshot.current_queue_depth < 1000;
    
    std::cout << "Status: " << (healthy ? "✓ HEALTHY" : "✗ DEGRADED") << "\n";
    
    if (!healthy) {
        std::cout << "\nAlerts:\n";
        if (snapshot.messages_dropped >= 10) {
            std::cout << "  ⚠ High drop rate detected\n";
        }
        if (snapshot.avg_log_latency_us >= 100) {
            std::cout << "  ⚠ High latency detected\n";
        }
        if (snapshot.current_queue_depth >= 1000) {
            std::cout << "  ⚠ Queue backlog detected\n";
        }
    }
}

int main() {
    std::cout << "Zyrnix Metrics & Observability Examples\n";
    std::cout << "======================================\n";
    
    example_basic_metrics();
    example_prometheus_export();
    example_json_export();
    example_sink_metrics();
    example_global_registry();
    example_monitoring_endpoint();
    example_production_monitoring();
    
    std::cout << "\n=== Key Benefits ===\n";
    std::cout << "1. Built-in observability for logging infrastructure\n";
    std::cout << "2. Prometheus integration for Grafana dashboards\n";
    std::cout << "3. JSON export for custom monitoring tools\n";
    std::cout << "4. Per-logger and per-sink granular metrics\n";
    std::cout << "5. Real-time health checks and alerting\n";
    std::cout << "6. Performance tracking (latency, throughput)\n";
    std::cout << "7. Queue depth monitoring for async logging\n";
    
    std::cout << "\n=== Grafana Dashboard Ideas ===\n";
    std::cout << "- Log throughput over time (messages/sec)\n";
    std::cout << "- Drop rate percentage\n";
    std::cout << "- P50/P95/P99 latency percentiles\n";
    std::cout << "- Queue depth heatmap\n";
    std::cout << "- Error rate by logger\n";
    std::cout << "- Bytes written by sink\n";
    
    return 0;
}
