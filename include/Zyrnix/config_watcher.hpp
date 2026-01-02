#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>

namespace Zyrnix {

class ConfigWatcher {
public:
    ConfigWatcher(const std::string& config_path, std::function<void()> on_change, std::chrono::milliseconds poll_interval = std::chrono::milliseconds(1000));
    ~ConfigWatcher();
    void start();
    void stop();
private:
    void watch_loop();
    std::string config_path_;
    std::function<void()> on_change_;
    std::chrono::milliseconds poll_interval_;
    std::atomic<bool> running_{false};
    std::thread watcher_thread_;
    time_t last_mtime_ = 0;
};

}
