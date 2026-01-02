#include <Zyrnix/sinks/loki_sink.hpp>
#include <Zyrnix/formatter.hpp>
#include <Zyrnix/log_message.hpp>
#include <Zyrnix/log_metrics.hpp>
#include <sstream>
#include <iostream>
#include <thread>
#include <curl/curl.h>

namespace Zyrnix {

LokiSink::LokiSink(const std::string& url, const std::string& labels, const LokiOptions& opts)
    : url_(url), labels_(labels), options_(opts) {
    last_flush_time_ = std::chrono::system_clock::now();
}

void LokiSink::set_options(const LokiOptions& opts) {
    std::lock_guard<std::mutex> lock(mutex_);
    options_ = opts;
}

void LokiSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::system_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

    std::ostringstream oss;
    oss << "{";
    oss << "\"ts\":\"" << ts << "\",";
    // Attach logger name and level as Loki entry fields in addition to the raw message
    oss << "\"logger\":\"" << logger_name << "\",";
    oss << "\"level\":\"" << to_string(level) << "\",";
    oss << "\"line\":\"" << message << "\"";
    oss << "}";

    buffer_.push_back(oss.str());

    const bool size_trigger = buffer_.size() >= options_.batch_size;
    const bool time_trigger = options_.flush_interval_ms > 0 &&
        std::chrono::duration_cast<std::chrono::milliseconds>(now - last_flush_time_).count() >=
            static_cast<long long>(options_.flush_interval_ms);

    if (size_trigger || time_trigger) {
        send_batch();
        last_flush_time_ = std::chrono::system_clock::now();
    }
}

void LokiSink::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!buffer_.empty()) {
        send_batch();
        last_flush_time_ = std::chrono::system_clock::now();
    }
}

void LokiSink::send_batch() {
    if (buffer_.empty()) return;

    std::ostringstream payload;
    payload << "{" << "\"streams\":[{" << "\"labels\":\"" << labels_ << "\"," << "\"entries\":[";
    for (size_t i = 0; i < buffer_.size(); ++i) {
        if (i > 0) payload << ",";
        payload << buffer_[i];
    }
    payload << "]}]}";

    // Basic retry with exponential backoff (v1.1.3)
    const int max_retries = 3;
    const long base_delay_ms = 100;

    auto& registry = MetricsRegistry::instance();
    auto sink_metrics = registry.get_sink_metrics(name_str());

    int attempt = 0;
    CURLcode last_err = CURLE_OK;

    while (attempt < max_retries) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            last_err = CURLE_FAILED_INIT;
            break;
        }

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.str().c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        if (options_.timeout_ms > 0) {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, options_.timeout_ms);
        }

        if (!options_.ca_cert_path.empty()) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, options_.ca_cert_path.c_str());
        }

        if (options_.insecure_skip_verify) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        last_err = curl_easy_perform(curl);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (last_err == CURLE_OK) {
            if (sink_metrics) {
                sink_metrics->record_flush();
            }
            buffer_.clear();
            return;
        }

        if (sink_metrics) {
            sink_metrics->record_error();
        }

        std::cerr << "LokiSink: send_batch attempt " << (attempt + 1)
                  << " failed: " << curl_easy_strerror(last_err) << std::endl;

        ++attempt;
        if (attempt < max_retries) {
            long delay = base_delay_ms * (1L << (attempt - 1));
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }

    // If we reach here, all retries failed – drop the batch to avoid unbounded growth
    buffer_.clear();
}

} // namespace Zyrnix
