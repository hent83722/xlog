#include "Zyrnix/log_metrics.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Zyrnix {


LogMetrics::LogMetrics()
    : start_time_(std::chrono::steady_clock::now())
{
}

void LogMetrics::record_message_logged() {
    counters_.messages_logged.fetch_add(1, std::memory_order_relaxed);
}

void LogMetrics::record_message_dropped() {
    counters_.messages_dropped.fetch_add(1, std::memory_order_relaxed);
}

void LogMetrics::record_message_filtered() {
    counters_.messages_filtered.fetch_add(1, std::memory_order_relaxed);
}

void LogMetrics::record_flush() {
    counters_.flushes.fetch_add(1, std::memory_order_relaxed);
}

void LogMetrics::record_error() {
    counters_.errors.fetch_add(1, std::memory_order_relaxed);
}

void LogMetrics::record_log_duration(uint64_t microseconds) {
    timings_.total_log_time_us.fetch_add(microseconds, std::memory_order_relaxed);
    
    uint64_t current_max = timings_.max_log_latency_us.load(std::memory_order_relaxed);
    while (microseconds > current_max) {
        if (timings_.max_log_latency_us.compare_exchange_weak(current_max, microseconds, std::memory_order_relaxed)) {
            break;
        }
    }
}

void LogMetrics::record_flush_duration(uint64_t microseconds) {
    timings_.total_flush_time_us.fetch_add(microseconds, std::memory_order_relaxed);
    
    uint64_t current_max = timings_.max_flush_latency_us.load(std::memory_order_relaxed);
    while (microseconds > current_max) {
        if (timings_.max_flush_latency_us.compare_exchange_weak(current_max, microseconds, std::memory_order_relaxed)) {
            break;
        }
    }
}

void LogMetrics::update_queue_depth(size_t depth) {
    queue_metrics_.current_depth.store(depth, std::memory_order_relaxed);
    
    size_t current_max = queue_metrics_.max_depth.load(std::memory_order_relaxed);
    while (depth > current_max) {
        if (queue_metrics_.max_depth.compare_exchange_weak(current_max, depth, std::memory_order_relaxed)) {
            break;
        }
    }
}

double LogMetrics::get_messages_per_second() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed_seconds = std::chrono::duration<double>(now - start_time_).count();
    
    if (elapsed_seconds < 0.001) {
        return 0.0;
    }
    
    return static_cast<double>(counters_.messages_logged.load(std::memory_order_relaxed)) / elapsed_seconds;
}

double LogMetrics::get_average_log_latency_us() const {
    uint64_t total_time = timings_.total_log_time_us.load(std::memory_order_relaxed);
    uint64_t count = counters_.messages_logged.load(std::memory_order_relaxed);
    
    if (count == 0) {
        return 0.0;
    }
    
    return static_cast<double>(total_time) / static_cast<double>(count);
}

double LogMetrics::get_average_flush_latency_us() const {
    uint64_t total_time = timings_.total_flush_time_us.load(std::memory_order_relaxed);
    uint64_t count = counters_.flushes.load(std::memory_order_relaxed);
    
    if (count == 0) {
        return 0.0;
    }
    
    return static_cast<double>(total_time) / static_cast<double>(count);
}

void LogMetrics::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    counters_.messages_logged.store(0, std::memory_order_relaxed);
    counters_.messages_dropped.store(0, std::memory_order_relaxed);
    counters_.messages_filtered.store(0, std::memory_order_relaxed);
    counters_.flushes.store(0, std::memory_order_relaxed);
    counters_.errors.store(0, std::memory_order_relaxed);
    
    timings_.total_log_time_us.store(0, std::memory_order_relaxed);
    timings_.total_flush_time_us.store(0, std::memory_order_relaxed);
    timings_.max_log_latency_us.store(0, std::memory_order_relaxed);
    timings_.max_flush_latency_us.store(0, std::memory_order_relaxed);
    
    queue_metrics_.current_depth.store(0, std::memory_order_relaxed);
    queue_metrics_.max_depth.store(0, std::memory_order_relaxed);
    
    start_time_ = std::chrono::steady_clock::now();
}

LogMetrics::Snapshot LogMetrics::get_snapshot() const {
    Snapshot snap;
    snap.messages_logged = get_messages_logged();
    snap.messages_dropped = get_messages_dropped();
    snap.messages_filtered = get_messages_filtered();
    snap.flushes = get_flushes();
    snap.errors = get_errors();
    snap.messages_per_second = get_messages_per_second();
    snap.avg_log_latency_us = get_average_log_latency_us();
    snap.avg_flush_latency_us = get_average_flush_latency_us();
    snap.max_log_latency_us = get_max_log_latency_us();
    snap.max_flush_latency_us = get_max_flush_latency_us();
    snap.current_queue_depth = get_current_queue_depth();
    snap.max_queue_depth = get_max_queue_depth();
    snap.timestamp = std::chrono::steady_clock::now();
    
    return snap;
}

std::string LogMetrics::export_prometheus(const std::string& prefix) const {
    std::ostringstream out;
    
    out << "# HELP " << prefix << "_messages_logged_total Total number of messages logged\n"
        << "# TYPE " << prefix << "_messages_logged_total counter\n"
        << prefix << "_messages_logged_total " << get_messages_logged() << "\n\n";
    
    out << "# HELP " << prefix << "_messages_dropped_total Total number of messages dropped\n"
        << "# TYPE " << prefix << "_messages_dropped_total counter\n"
        << prefix << "_messages_dropped_total " << get_messages_dropped() << "\n\n";
    
    out << "# HELP " << prefix << "_messages_filtered_total Total number of messages filtered\n"
        << "# TYPE " << prefix << "_messages_filtered_total counter\n"
        << prefix << "_messages_filtered_total " << get_messages_filtered() << "\n\n";
    
    out << "# HELP " << prefix << "_messages_per_second Current logging rate\n"
        << "# TYPE " << prefix << "_messages_per_second gauge\n"
        << prefix << "_messages_per_second " << std::fixed << std::setprecision(2) 
        << get_messages_per_second() << "\n\n";
    
    out << "# HELP " << prefix << "_log_latency_us_avg Average log call latency in microseconds\n"
        << "# TYPE " << prefix << "_log_latency_us_avg gauge\n"
        << prefix << "_log_latency_us_avg " << std::fixed << std::setprecision(2)
        << get_average_log_latency_us() << "\n\n";
    
    out << "# HELP " << prefix << "_log_latency_us_max Maximum log call latency in microseconds\n"
        << "# TYPE " << prefix << "_log_latency_us_max gauge\n"
        << prefix << "_log_latency_us_max " << get_max_log_latency_us() << "\n\n";
    
    out << "# HELP " << prefix << "_queue_depth Current async queue depth\n"
        << "# TYPE " << prefix << "_queue_depth gauge\n"
        << prefix << "_queue_depth " << get_current_queue_depth() << "\n\n";
    
    out << "# HELP " << prefix << "_queue_depth_max Maximum async queue depth\n"
        << "# TYPE " << prefix << "_queue_depth_max gauge\n"
        << prefix << "_queue_depth_max " << get_max_queue_depth() << "\n\n";
    
    out << "# HELP " << prefix << "_errors_total Total number of logging errors\n"
        << "# TYPE " << prefix << "_errors_total counter\n"
        << prefix << "_errors_total " << get_errors() << "\n\n";
    
    return out.str();
}

std::string LogMetrics::export_json() const {
    std::ostringstream json;
    
    json << "{"
         << "\"messages_logged\":" << get_messages_logged() << ","
         << "\"messages_dropped\":" << get_messages_dropped() << ","
         << "\"messages_filtered\":" << get_messages_filtered() << ","
         << "\"flushes\":" << get_flushes() << ","
         << "\"errors\":" << get_errors() << ","
         << "\"messages_per_second\":" << std::fixed << std::setprecision(2) << get_messages_per_second() << ","
         << "\"avg_log_latency_us\":" << std::fixed << std::setprecision(2) << get_average_log_latency_us() << ","
         << "\"avg_flush_latency_us\":" << std::fixed << std::setprecision(2) << get_average_flush_latency_us() << ","
         << "\"max_log_latency_us\":" << get_max_log_latency_us() << ","
         << "\"max_flush_latency_us\":" << get_max_flush_latency_us() << ","
         << "\"current_queue_depth\":" << get_current_queue_depth() << ","
         << "\"max_queue_depth\":" << get_max_queue_depth()
         << "}";
    
    return json.str();
}


SinkMetrics::SinkMetrics(const std::string& sink_name)
    : name_(sink_name)
{
}

void SinkMetrics::record_write(size_t bytes) {
    writes_.fetch_add(1, std::memory_order_relaxed);
    bytes_written_.fetch_add(bytes, std::memory_order_relaxed);
}

void SinkMetrics::record_flush() {
    flushes_.fetch_add(1, std::memory_order_relaxed);
}

void SinkMetrics::record_error() {
    errors_.fetch_add(1, std::memory_order_relaxed);
}

void SinkMetrics::record_write_duration(uint64_t microseconds) {
    total_write_time_us_.fetch_add(microseconds, std::memory_order_relaxed);
}

double SinkMetrics::get_average_write_latency_us() const {
    uint64_t total_time = total_write_time_us_.load(std::memory_order_relaxed);
    uint64_t count = writes_.load(std::memory_order_relaxed);
    
    if (count == 0) {
        return 0.0;
    }
    
    return static_cast<double>(total_time) / static_cast<double>(count);
}

std::string SinkMetrics::export_prometheus(const std::string& prefix) const {
    std::ostringstream out;
    
    out << "# HELP " << prefix << "_sink_writes_total Total writes by sink\n"
        << "# TYPE " << prefix << "_sink_writes_total counter\n"
        << prefix << "_sink_writes_total{sink=\"" << name_ << "\"} " << get_writes() << "\n\n";
    
    out << "# HELP " << prefix << "_sink_bytes_written_total Total bytes written by sink\n"
        << "# TYPE " << prefix << "_sink_bytes_written_total counter\n"
        << prefix << "_sink_bytes_written_total{sink=\"" << name_ << "\"} " << get_bytes_written() << "\n\n";
    
    out << "# HELP " << prefix << "_sink_write_latency_us_avg Average write latency by sink\n"
        << "# TYPE " << prefix << "_sink_write_latency_us_avg gauge\n"
        << prefix << "_sink_write_latency_us_avg{sink=\"" << name_ << "\"} " 
        << std::fixed << std::setprecision(2) << get_average_write_latency_us() << "\n\n";
    
    return out.str();
}


MetricsRegistry& MetricsRegistry::instance() {
    static MetricsRegistry registry;
    return registry;
}

std::shared_ptr<LogMetrics> MetricsRegistry::get_logger_metrics(const std::string& logger_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = logger_metrics_.find(logger_name);
    if (it != logger_metrics_.end()) {
        return it->second;
    }
    
    auto metrics = std::make_shared<LogMetrics>();
    logger_metrics_[logger_name] = metrics;
    return metrics;
}

std::shared_ptr<SinkMetrics> MetricsRegistry::get_sink_metrics(const std::string& sink_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sink_metrics_.find(sink_name);
    if (it != sink_metrics_.end()) {
        return it->second;
    }
    
    auto metrics = std::make_shared<SinkMetrics>(sink_name);
    sink_metrics_[sink_name] = metrics;
    return metrics;
}

std::map<std::string, LogMetrics::Snapshot> MetricsRegistry::get_all_logger_snapshots() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::map<std::string, LogMetrics::Snapshot> snapshots;
    for (const auto& pair : logger_metrics_) {
        snapshots[pair.first] = pair.second->get_snapshot();
    }
    
    return snapshots;
}

std::string MetricsRegistry::export_all_prometheus(const std::string& prefix) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ostringstream out;
    
    for (const auto& pair : logger_metrics_) {
        out << "# Logger: " << pair.first << "\n";
        out << pair.second->export_prometheus(prefix + "_logger");
    }
    
    for (const auto& pair : sink_metrics_) {
        out << pair.second->export_prometheus(prefix);
    }
    
    return out.str();
}

std::string MetricsRegistry::export_all_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ostringstream json;
    json << "{\"loggers\":{";
    
    bool first_logger = true;
    for (const auto& pair : logger_metrics_) {
        if (!first_logger) json << ",";
        json << "\"" << pair.first << "\":" << pair.second->export_json();
        first_logger = false;
    }
    
    json << "},\"sinks\":{";
    
    bool first_sink = true;
    for (const auto& pair : sink_metrics_) {
        if (!first_sink) json << ",";
        json << "\"" << pair.first << "\":{"
             << "\"writes\":" << pair.second->get_writes() << ","
             << "\"bytes_written\":" << pair.second->get_bytes_written() << ","
             << "\"flushes\":" << pair.second->get_flushes() << ","
             << "\"errors\":" << pair.second->get_errors() << ","
             << "\"avg_write_latency_us\":" << std::fixed << std::setprecision(2) 
             << pair.second->get_average_write_latency_us()
             << "}";
        first_sink = false;
    }
    
    json << "}}";
    return json.str();
}

void MetricsRegistry::reset_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pair : logger_metrics_) {
        pair.second->reset();
    }
}

} // namespace Zyrnix
