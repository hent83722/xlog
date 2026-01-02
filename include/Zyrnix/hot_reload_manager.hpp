#pragma once
#include <map>
#include <memory>
#include <string>
#include <mutex>
#include <atomic>
#include <chrono>
#include "Zyrnix/config.hpp"
#include "Zyrnix/config_watcher.hpp"

namespace Zyrnix {

class HotReloadManager {
public:
    HotReloadManager(const std::string& config_path);
    ~HotReloadManager();
    void start();
    void stop();
    std::shared_ptr<Logger> get_logger(const std::string& name);
    std::map<std::string, std::shared_ptr<Logger>> get_all_loggers();

    // Hot-reload metrics (v1.1.3)
    uint64_t reload_success_count() const { return reload_successes_.load(std::memory_order_relaxed); }
    uint64_t reload_failure_count() const { return reload_failures_.load(std::memory_order_relaxed); }
    std::chrono::system_clock::time_point last_reload_time() const { return last_reload_time_; }
private:
    void reload();
    std::string config_path_;
    std::map<std::string, std::shared_ptr<Logger>> loggers_;
    std::mutex mtx_;
    std::unique_ptr<ConfigWatcher> watcher_;
    std::atomic<uint64_t> reload_successes_{0};
    std::atomic<uint64_t> reload_failures_{0};
    std::chrono::system_clock::time_point last_reload_time_{};
};

}
