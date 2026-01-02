#include "Zyrnix/config_watcher.hpp"
#include <sys/stat.h>

namespace Zyrnix {

ConfigWatcher::ConfigWatcher(const std::string& config_path, std::function<void()> on_change, std::chrono::milliseconds poll_interval)
    : config_path_(config_path), on_change_(on_change), poll_interval_(poll_interval) {}

ConfigWatcher::~ConfigWatcher() {
    stop();
}

void ConfigWatcher::start() {
    if (running_) return;
    running_ = true;
    watcher_thread_ = std::thread(&ConfigWatcher::watch_loop, this);
}

void ConfigWatcher::stop() {
    running_ = false;
    if (watcher_thread_.joinable()) watcher_thread_.join();
}

void ConfigWatcher::watch_loop() {
    while (running_) {
        struct stat st;
        if (stat(config_path_.c_str(), &st) == 0) {
            if (last_mtime_ == 0) last_mtime_ = st.st_mtime;
            if (st.st_mtime != last_mtime_) {
                last_mtime_ = st.st_mtime;
                if (on_change_) on_change_();
            }
        }
        std::this_thread::sleep_for(poll_interval_);
    }
}

} // namespace Zyrnix
