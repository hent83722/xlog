#include "Zyrnix/hot_reload_manager.hpp"
#include <iostream>

namespace Zyrnix {

HotReloadManager::HotReloadManager(const std::string& config_path)
    : config_path_(config_path) {}

HotReloadManager::~HotReloadManager() {
    stop();
}

void HotReloadManager::start() {
    watcher_ = std::make_unique<ConfigWatcher>(config_path_, [this]() { reload(); });
    reload();
    watcher_->start();
}

void HotReloadManager::stop() {
    if (watcher_) watcher_->stop();
}

void HotReloadManager::reload() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ConfigLoader::load_from_json(config_path_)) {
        ++reload_failures_;
        std::cerr << "Failed to reload config: " << config_path_;
        std::string reason = ConfigLoader::get_last_error();
        if (!reason.empty()) {
            std::cerr << " (reason: " << reason << ")";
        }
        std::cerr << std::endl;
        return;
    }

    loggers_ = ConfigLoader::create_loggers();
    last_reload_time_ = std::chrono::system_clock::now();
    ++reload_successes_;
    std::cout << "Reloaded logger configuration from: " << config_path_ << std::endl;
}

std::shared_ptr<Logger> HotReloadManager::get_logger(const std::string& name) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = loggers_.find(name);
    return (it != loggers_.end()) ? it->second : nullptr;
}

std::map<std::string, std::shared_ptr<Logger>> HotReloadManager::get_all_loggers() {
    std::lock_guard<std::mutex> lock(mtx_);
    return loggers_;
}

} // namespace Zyrnix
