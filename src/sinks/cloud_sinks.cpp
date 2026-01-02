#include "Zyrnix/sinks/cloud_sinks.hpp"
#include "Zyrnix/log_record.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

#ifdef XLOG_HAS_CURL
#include <curl/curl.h>
#endif

namespace Zyrnix {


CloudWatchSink::CloudWatchSink(const Config& config)
    : config_(config)
    , running_(true)
    , messages_sent_(0)
    , messages_failed_(0)
    , messages_dropped_(0)
    , batches_sent_(0)
    , retries_(0)
{
    worker_ = std::thread(&CloudWatchSink::worker_thread, this);
}

CloudWatchSink::~CloudWatchSink() {
    running_ = false;
    queue_cv_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }
}

void CloudWatchSink::log(const std::string& name, LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    if (queue_.size() >= config_.max_queue_size) {
        messages_dropped_++;
        return;
    }

    LogEvent event;
    event.message = formatter.format(name, level, message);
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    queue_.push(event);
    queue_cv_.notify_one();
}

void CloudWatchSink::flush() {
    while (true) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (queue_.empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void CloudWatchSink::worker_thread() {
    std::vector<LogEvent> batch;
    auto last_send = std::chrono::steady_clock::now();

    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        queue_cv_.wait_for(lock, std::chrono::milliseconds(100), [this] {
            return !queue_.empty() || !running_;
        });

        if (!running_ && queue_.empty()) {
            break;
        }

        while (!queue_.empty() && batch.size() < config_.batch_size) {
            batch.push_back(queue_.front());
            queue_.pop();
        }

        lock.unlock();

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_send).count();
        
        if (!batch.empty() && (batch.size() >= config_.batch_size || elapsed >= static_cast<long>(config_.batch_timeout_ms))) {
            send_batch(batch);
            batch.clear();
            last_send = now;
        }
    }

    if (!batch.empty()) {
        send_batch(batch);
    }
}

void CloudWatchSink::send_batch(const std::vector<LogEvent>& events) {
    size_t retry_count = 0;
    size_t delay = config_.retry_delay_ms;
    bool success = false;

    while (retry_count <= config_.max_retries && !success) {
        success = send_to_cloudwatch(events);
        
        if (!success && retry_count < config_.max_retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            delay *= 2;
            retry_count++;
            
            std::lock_guard<std::mutex> lock(stats_mutex_);
            retries_++;
        }
    }

    std::lock_guard<std::mutex> lock(stats_mutex_);
    batches_sent_++;
    
    if (success) {
        messages_sent_ += events.size();
    } else {
        messages_failed_ += events.size();
    }
}

bool CloudWatchSink::send_to_cloudwatch(const std::vector<LogEvent>& events) {
    if (events.empty()) {
        return true;
    }

    std::string body = create_request_body(events);
    std::string url = "https://logs." + config_.region + ".amazonaws.com/";

    std::vector<std::pair<std::string, std::string>> headers = {
        {"Content-Type", "application/x-amz-json-1.1"},
        {"X-Amz-Target", "Logs_20140328.PutLogEvents"}
    };

    auto response = HttpClient::post(url, body, headers);
    return response.success && response.status_code == 200;
}

std::string CloudWatchSink::create_request_body(const std::vector<LogEvent>& events) {
    std::ostringstream json;
    json << "{"
         << "\"logGroupName\":\"" << config_.log_group_name << "\","
         << "\"logStreamName\":\"" << config_.log_stream_name << "\","
         << "\"logEvents\":[";

    for (size_t i = 0; i < events.size(); ++i) {
        if (i > 0) json << ",";
        json << "{"
             << "\"timestamp\":" << events[i].timestamp_ms << ","
             << "\"message\":\"" << events[i].message << "\""
             << "}";
    }

    json << "]}";
    return json.str();
}
CloudWatchSink::Stats CloudWatchSink::get_stats() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(stats_mutex_));
    std::lock_guard<std::mutex> queue_lock(const_cast<std::mutex&>(queue_mutex_));
    
    Stats stats;
    stats.messages_sent = messages_sent_;
    stats.messages_failed = messages_failed_;
    stats.messages_dropped = messages_dropped_;
    stats.batches_sent = batches_sent_;
    stats.retries = retries_;
    stats.queue_size = queue_.size();
    
    return stats;
}


AzureMonitorSink::AzureMonitorSink(const Config& config)
    : config_(config)
    , running_(true)
    , messages_sent_(0)
    , messages_failed_(0)
    , messages_dropped_(0)
    , batches_sent_(0)
    , retries_(0)
{
    worker_ = std::thread(&AzureMonitorSink::worker_thread, this);
}

AzureMonitorSink::~AzureMonitorSink() {
    running_ = false;
    queue_cv_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }
}

void AzureMonitorSink::log(const std::string& name, LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    if (queue_.size() >= config_.max_queue_size) {
        messages_dropped_++;
        return;
    }

    TelemetryEvent event;
    event.message = formatter.format(name, level, message);
    event.level = level_to_severity(level);
    event.logger_name = name;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    gmtime_r(&time_t, &tm);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << "Z";
    event.timestamp = oss.str();

    queue_.push(event);
    queue_cv_.notify_one();
}

void AzureMonitorSink::flush() {
    while (true) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (queue_.empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void AzureMonitorSink::worker_thread() {
    std::vector<TelemetryEvent> batch;
    auto last_send = std::chrono::steady_clock::now();

    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        queue_cv_.wait_for(lock, std::chrono::milliseconds(100), [this] {
            return !queue_.empty() || !running_;
        });

        if (!running_ && queue_.empty()) {
            break;
        }

        while (!queue_.empty() && batch.size() < config_.batch_size) {
            batch.push_back(queue_.front());
            queue_.pop();
        }

        lock.unlock();

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_send).count();
        
        if (!batch.empty() && (batch.size() >= config_.batch_size || elapsed >= static_cast<long>(config_.batch_timeout_ms))) {
            send_batch(batch);
            batch.clear();
            last_send = now;
        }
    }

    if (!batch.empty()) {
        send_batch(batch);
    }
}

void AzureMonitorSink::send_batch(const std::vector<TelemetryEvent>& events) {
    size_t retry_count = 0;
    size_t delay = config_.retry_delay_ms;
    bool success = false;

    while (retry_count <= config_.max_retries && !success) {
        success = send_to_azure(events);
        
        if (!success && retry_count < config_.max_retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            delay *= 2;
            retry_count++;
            
            std::lock_guard<std::mutex> lock(stats_mutex_);
            retries_++;
        }
    }

    std::lock_guard<std::mutex> lock(stats_mutex_);
    batches_sent_++;
    
    if (success) {
        messages_sent_ += events.size();
    } else {
        messages_failed_ += events.size();
    }
}

bool AzureMonitorSink::send_to_azure(const std::vector<TelemetryEvent>& events) {
    if (events.empty()) {
        return true;
    }

    std::string body = create_request_body(events);
    
    std::vector<std::pair<std::string, std::string>> headers = {
        {"Content-Type", "application/json"},
        {"charset", "utf-8"}
    };

    auto response = HttpClient::post(config_.ingestion_endpoint, body, headers);
    return response.success && (response.status_code == 200 || response.status_code == 206);
}

std::string AzureMonitorSink::create_request_body(const std::vector<TelemetryEvent>& events) {
    std::ostringstream json;
    
    for (const auto& event : events) {
        json << "{"
             << "\"name\":\"Microsoft.ApplicationInsights.Message\","
             << "\"time\":\"" << event.timestamp << "\","
             << "\"iKey\":\"" << config_.instrumentation_key << "\","
             << "\"data\":{"
             << "\"baseType\":\"MessageData\","
             << "\"baseData\":{"
             << "\"ver\":2,"
             << "\"message\":\"" << event.message << "\","
             << "\"severityLevel\":\"" << event.level << "\""
             << "}";
        
        if (!config_.cloud_role_name.empty()) {
            json << ",\"cloud\":{\"roleName\":\"" << config_.cloud_role_name << "\"}";
        }
        
        json << "}}\n";
    }
    
    return json.str();
}

std::string AzureMonitorSink::level_to_severity(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:
        case LogLevel::Debug:
            return "Verbose";
        case LogLevel::Info:
            return "Information";
        case LogLevel::Warn:
            return "Warning";
        case LogLevel::Error:
            return "Error";
        case LogLevel::Critical:
            return "Critical";
        default:
            return "Information";
    }
}

AzureMonitorSink::Stats AzureMonitorSink::get_stats() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(stats_mutex_));
    std::lock_guard<std::mutex> queue_lock(const_cast<std::mutex&>(queue_mutex_));
    
    Stats stats;
    stats.messages_sent = messages_sent_;
    stats.messages_failed = messages_failed_;
    stats.messages_dropped = messages_dropped_;
    stats.batches_sent = batches_sent_;
    stats.retries = retries_;
    stats.queue_size = queue_.size();
    
    return stats;
}


#ifdef XLOG_HAS_CURL
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
#endif

HttpClient::Response HttpClient::post(
    const std::string& url,
    const std::string& body,
    const std::vector<std::pair<std::string, std::string>>& headers
) {
    Response response;
    response.success = false;
    response.status_code = 0;

#ifdef XLOG_HAS_CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        return response;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);

    struct curl_slist* header_list = nullptr;
    for (const auto& header : headers) {
        std::string header_str = header.first + ": " + header.second;
        header_list = curl_slist_append(header_list, header_str.c_str());
    }
    if (header_list) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    }

    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        long status_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        response.status_code = static_cast<int>(status_code);
        response.success = true;
    }

    if (header_list) {
        curl_slist_free_all(header_list);
    }
    curl_easy_cleanup(curl);
#else
    std::string cmd = "curl -X POST -d '" + body + "' '" + url + "'";
    for (const auto& header : headers) {
        cmd += " -H '" + header.first + ": " + header.second + "'";
    }
    cmd += " -w '%{http_code}' -s -o /tmp/Zyrnix_http_response 2>/dev/null";
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[8];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            response.status_code = std::atoi(buffer);
            response.success = (response.status_code >= 200 && response.status_code < 300);
        }
        pclose(pipe);
    }
#endif

    return response;
}

bool HttpClient::is_available() {
#ifdef XLOG_HAS_CURL
    return true;
#else
    return std::system("which curl > /dev/null 2>&1") == 0;
#endif
}

} // namespace Zyrnix
