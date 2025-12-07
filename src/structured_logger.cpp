#include "xlog/structured_logger.hpp"

namespace xlog {

std::shared_ptr<StructuredLogger> StructuredLogger::create(const std::string& name, const std::string& filename) {
    auto logger = std::make_shared<Logger>(name);
    auto sink = std::make_shared<StructuredJsonSink>(filename);
    logger->add_sink(sink);
    return std::make_shared<StructuredLogger>(logger, sink);
}

StructuredLogger::StructuredLogger(std::shared_ptr<Logger> logger_, std::shared_ptr<StructuredJsonSink> sink_)
    : logger(logger_), json_sink(sink_) {}

void StructuredLogger::set_context(const std::string& key, const std::string& value) {
    json_sink->set_context(key, value);
}

void StructuredLogger::clear_context() {
    json_sink->clear_context();
}

void StructuredLogger::trace(const std::string& message, const std::map<std::string, std::string>& fields) {
    json_sink->log_with_fields(logger->name, LogLevel::Trace, message, fields);
}

void StructuredLogger::debug(const std::string& message, const std::map<std::string, std::string>& fields) {
    json_sink->log_with_fields(logger->name, LogLevel::Debug, message, fields);
}

void StructuredLogger::info(const std::string& message, const std::map<std::string, std::string>& fields) {
    json_sink->log_with_fields(logger->name, LogLevel::Info, message, fields);
}

void StructuredLogger::warn(const std::string& message, const std::map<std::string, std::string>& fields) {
    json_sink->log_with_fields(logger->name, LogLevel::Warn, message, fields);
}

void StructuredLogger::error(const std::string& message, const std::map<std::string, std::string>& fields) {
    json_sink->log_with_fields(logger->name, LogLevel::Error, message, fields);
}

void StructuredLogger::critical(const std::string& message, const std::map<std::string, std::string>& fields) {
    json_sink->log_with_fields(logger->name, LogLevel::Critical, message, fields);
}

}
