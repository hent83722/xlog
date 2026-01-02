#pragma once
#include "Zyrnix_features.hpp"
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <atomic>
#include <mutex>
#include <chrono>
#include <memory>

namespace Zyrnix {

class LogMetrics {
public:
    struct Counters {
        std::atomic<uint64_t> messages_logged{0};
        std::atomic<uint64_t> messages_dropped{0};
        std::atomic<uint64_t> messages_filtered{0};
        std::atomic<uint64_t> flushes{0};
        std::atomic<uint64_t> errors{0};
    };

    struct Timings {
        std::atomic<uint64_t> total_log_time_us{0};  // Total time spent logging (microseconds)
        std::atomic<uint64_t> total_flush_time_us{0}; // Total time spent flushing
        std::atomic<uint64_t> max_log_latency_us{0};  // Max single log call latency
        std::atomic<uint64_t> max_flush_latency_us{0}; // Max single flush latency
    };

    struct QueueMetrics {
        std::atomic<size_t> current_depth{0};
        std::atomic<size_t> max_depth{0};
        std::atomic<uint64_t> enqueue_count{0};
        std::atomic<uint64_t> dequeue_count{0};
    };

    LogMetrics();

    void record_message_logged();
    void record_message_dropped();
    void record_message_filtered();
    void record_flush();
    void record_error();
    void record_log_duration(uint64_t microseconds);
    void record_flush_duration(uint64_t microseconds);
    void update_queue_depth(size_t depth);

    uint64_t get_messages_logged() const { return counters_.messages_logged.load(std::memory_order_relaxed); }
    uint64_t get_messages_dropped() const { return counters_.messages_dropped.load(std::memory_order_relaxed); }
    uint64_t get_messages_filtered() const { return counters_.messages_filtered.load(std::memory_order_relaxed); }
    uint64_t get_flushes() const { return counters_.flushes.load(std::memory_order_relaxed); }
    uint64_t get_errors() const { return counters_.errors.load(std::memory_order_relaxed); }
    
    double get_messages_per_second() const;
    double get_average_log_latency_us() const;
    double get_average_flush_latency_us() const;
    uint64_t get_max_log_latency_us() const { return timings_.max_log_latency_us.load(std::memory_order_relaxed); }
    uint64_t get_max_flush_latency_us() const { return timings_.max_flush_latency_us.load(std::memory_order_relaxed); }
    
    size_t get_current_queue_depth() const { return queue_metrics_.current_depth.load(std::memory_order_relaxed); }
    size_t get_max_queue_depth() const { return queue_metrics_.max_depth.load(std::memory_order_relaxed); }

    void reset();

    struct Snapshot {
        uint64_t messages_logged;
        uint64_t messages_dropped;
        uint64_t messages_filtered;
        uint64_t flushes;
        uint64_t errors;
        double messages_per_second;
        double avg_log_latency_us;
        double avg_flush_latency_us;
        uint64_t max_log_latency_us;
        uint64_t max_flush_latency_us;
        size_t current_queue_depth;
        size_t max_queue_depth;
        std::chrono::steady_clock::time_point timestamp;
    };

    Snapshot get_snapshot() const;

    std::string export_prometheus(const std::string& prefix = "Zyrnix") const;

    std::string export_json() const;

private:
    Counters counters_;
    Timings timings_;
    QueueMetrics queue_metrics_;
    std::chrono::steady_clock::time_point start_time_;
    mutable std::mutex mutex_;
};

class SinkMetrics {
public:
    SinkMetrics(const std::string& sink_name);

    void record_write(size_t bytes);
    void record_flush();
    void record_error();
    void record_write_duration(uint64_t microseconds);

    std::string get_name() const { return name_; }
    uint64_t get_writes() const { return writes_.load(std::memory_order_relaxed); }
    uint64_t get_bytes_written() const { return bytes_written_.load(std::memory_order_relaxed); }
    uint64_t get_flushes() const { return flushes_.load(std::memory_order_relaxed); }
    uint64_t get_errors() const { return errors_.load(std::memory_order_relaxed); }
    double get_average_write_latency_us() const;

    std::string export_prometheus(const std::string& prefix = "Zyrnix") const;

private:
    std::string name_;
    std::atomic<uint64_t> writes_{0};
    std::atomic<uint64_t> bytes_written_{0};
    std::atomic<uint64_t> flushes_{0};
    std::atomic<uint64_t> errors_{0};
    std::atomic<uint64_t> total_write_time_us_{0};
};

class MetricsRegistry {
public:
    static MetricsRegistry& instance();

    std::shared_ptr<LogMetrics> get_logger_metrics(const std::string& logger_name);
    std::shared_ptr<SinkMetrics> get_sink_metrics(const std::string& sink_name);

    std::map<std::string, LogMetrics::Snapshot> get_all_logger_snapshots() const;

    std::string export_all_prometheus(const std::string& prefix = "Zyrnix") const;

    std::string export_all_json() const;

    void reset_all();

private:
    MetricsRegistry() = default;
    
    mutable std::mutex mutex_;
    std::map<std::string, std::shared_ptr<LogMetrics>> logger_metrics_;
    std::map<std::string, std::shared_ptr<SinkMetrics>> sink_metrics_;
};

class ScopedTimer {
public:
    using Callback = std::function<void(uint64_t)>;

    explicit ScopedTimer(Callback callback)
        : callback_(callback)
        , start_(std::chrono::steady_clock::now())
    {}

    ~ScopedTimer() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        callback_(duration);
    }

private:
    Callback callback_;
    std::chrono::steady_clock::time_point start_;
};

} // namespace Zyrnix
