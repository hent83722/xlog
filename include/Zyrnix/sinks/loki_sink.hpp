#pragma once
#include "../log_sink.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <chrono>

namespace Zyrnix {

struct LokiOptions {
    size_t batch_size = 10;
    uint64_t flush_interval_ms = 0;
    long timeout_ms = 5000;
    bool insecure_skip_verify = false;
    std::string ca_cert_path;
};

class LokiSink : public LogSink {
public:
    LokiSink(const std::string& url, const std::string& labels = "", const LokiOptions& opts = LokiOptions());
    void log(const std::string& name, LogLevel level, const std::string& message) override;
    void flush();
    const char* name_str() const noexcept { return "LokiSink"; }

    bool is_cloud_sink() const override { return true; }

    void set_options(const LokiOptions& opts);

private:
    std::string url_;
    std::string labels_;
    LokiOptions options_;
    std::vector<std::string> buffer_;
    std::mutex mutex_;
    std::chrono::system_clock::time_point last_flush_time_{};

    void send_batch();
};

using LokiSinkPtr = std::shared_ptr<LokiSink>;

} // namespace Zyrnix
