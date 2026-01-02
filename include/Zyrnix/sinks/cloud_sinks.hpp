#pragma once
#include "../Zyrnix_features.hpp"
#include "../log_sink.hpp"
#include "../log_record.hpp"
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>

namespace Zyrnix {

class CloudWatchSink : public LogSink {
public:
    struct Config {
        std::string region = "us-east-1";
        std::string log_group_name;
        std::string log_stream_name;
        std::string access_key_id;
        std::string secret_access_key;
        
        size_t batch_size = 100; // Max messages per batch
        size_t batch_timeout_ms = 5000; // Max time to wait before sending batch
        size_t max_retries = 3; // Retry attempts on failure
        size_t retry_delay_ms = 1000; // Initial retry delay (exponential backoff)
        size_t max_queue_size = 10000; // Max messages in queue
    };

    explicit CloudWatchSink(const Config& config);
    ~CloudWatchSink() override;

    void log(const std::string& name, LogLevel level, const std::string& message) override;
    void flush();

    bool is_cloud_sink() const override { return true; }

    struct Stats {
        uint64_t messages_sent;
        uint64_t messages_failed;
        uint64_t messages_dropped;
        uint64_t batches_sent;
        uint64_t retries;
        size_t queue_size;
    };

    Stats get_stats() const;

private:
    struct LogEvent {
        std::string message;
        int64_t timestamp_ms;
    };

    void worker_thread();
    void send_batch(const std::vector<LogEvent>& events);
    bool send_to_cloudwatch(const std::vector<LogEvent>& events);
    std::string create_request_body(const std::vector<LogEvent>& events);
    
    Config config_;
    
    std::queue<LogEvent> queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    std::thread worker_;
    std::atomic<bool> running_;
    
    mutable std::mutex stats_mutex_;
    uint64_t messages_sent_;
    uint64_t messages_failed_;
    uint64_t messages_dropped_;
    uint64_t batches_sent_;
    uint64_t retries_;
};

class AzureMonitorSink : public LogSink {
public:
    struct Config {
        std::string instrumentation_key;
        std::string ingestion_endpoint = "https://dc.services.visualstudio.com/v2/track";
        
        size_t batch_size = 100;
        size_t batch_timeout_ms = 5000;
        size_t max_retries = 3;
        size_t retry_delay_ms = 1000;
        size_t max_queue_size = 10000;
        
        std::string cloud_role_name;
        std::string cloud_role_instance;
    };

    explicit AzureMonitorSink(const Config& config);
    ~AzureMonitorSink() override;

    void log(const std::string& name, LogLevel level, const std::string& message) override;
    void flush();

    bool is_cloud_sink() const override { return true; }

    struct Stats {
        uint64_t messages_sent;
        uint64_t messages_failed;
        uint64_t messages_dropped;
        uint64_t batches_sent;
        uint64_t retries;
        size_t queue_size;
    };

    Stats get_stats() const;

private:
    struct TelemetryEvent {
        std::string message;
        std::string level;
        std::string timestamp;
        std::string logger_name;
    };

    void worker_thread();
    void send_batch(const std::vector<TelemetryEvent>& events);
    bool send_to_azure(const std::vector<TelemetryEvent>& events);
    std::string create_request_body(const std::vector<TelemetryEvent>& events);
    std::string level_to_severity(LogLevel level);
    
    Config config_;
    
    std::queue<TelemetryEvent> queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    std::thread worker_;
    std::atomic<bool> running_;
    
    mutable std::mutex stats_mutex_;
    uint64_t messages_sent_;
    uint64_t messages_failed_;
    uint64_t messages_dropped_;
    uint64_t batches_sent_;
    uint64_t retries_;
};

class HttpClient {
public:
    struct Response {
        int status_code;
        std::string body;
        bool success;
    };

    static Response post(
        const std::string& url,
        const std::string& body,
        const std::vector<std::pair<std::string, std::string>>& headers = {}
    );

    static bool is_available();
};

} // namespace Zyrnix
