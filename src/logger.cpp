#include "xlog/logger.hpp"
#include "xlog/log_sink.hpp"
#include <mutex>

namespace xlog {

Logger::Logger(std::string n) : name(std::move(n)) {}

void Logger::add_sink(LogSinkPtr sink) {
    std::lock_guard<std::mutex> lock(mtx);
    sinks.push_back(std::move(sink));
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& s : sinks) s->log(name, level, message);
}

void Logger::trace(const std::string& msg) { log(LogLevel::Trace, msg); }
void Logger::debug(const std::string& msg) { log(LogLevel::Debug, msg); }
void Logger::info(const std::string& msg) { log(LogLevel::Info, msg); }
void Logger::warn(const std::string& msg) { log(LogLevel::Warn, msg); }
void Logger::error(const std::string& msg) { log(LogLevel::Error, msg); }
void Logger::critical(const std::string& msg) { log(LogLevel::Critical, msg); }

}
