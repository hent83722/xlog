#include "Zyrnix/log_health.hpp"
#include "Zyrnix/logger.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Zyrnix {

std::string HealthCheckResult::to_json() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "{\n";
    oss << "  \"status\": \"";
    switch (status) {
        case HealthStatus::Healthy: oss << "healthy"; break;
        case HealthStatus::Degraded: oss << "degraded"; break;
        case HealthStatus::Unhealthy: oss << "unhealthy"; break;
    }
    oss << "\",\n";
    
    oss << "  \"message\": \"" << message << "\",\n";
    
    auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
    oss << "  \"timestamp\": \"" << std::put_time(std::gmtime(&time_t_val), "%Y-%m-%dT%H:%M:%SZ") << "\",\n";
    
    oss << "  \"metrics\": {\n";
    oss << "    \"messages_logged\": " << messages_logged << ",\n";
    oss << "    \"messages_dropped\": " << messages_dropped << ",\n";
    oss << "    \"messages_filtered\": " << messages_filtered << ",\n";
    oss << "    \"errors\": " << errors << ",\n";
    oss << "    \"messages_per_second\": " << messages_per_second << ",\n";
    oss << "    \"avg_latency_us\": " << avg_latency_us << ",\n";
    oss << "    \"max_latency_us\": " << max_latency_us << ",\n";
    oss << "    \"queue_depth\": " << queue_depth << ",\n";
    oss << "    \"max_queue_depth\": " << max_queue_depth << "\n";
    oss << "  },\n";
    
    oss << "  \"indicators\": {\n";
    oss << "    \"drop_rate\": " << (drop_rate * 100.0) << ",\n";
    oss << "    \"error_rate\": " << (error_rate * 100.0) << ",\n";
    oss << "    \"queue_full_warning\": " << (queue_full_warning ? "true" : "false") << ",\n";
    oss << "    \"high_latency_warning\": " << (high_latency_warning ? "true" : "false") << ",\n";
    
    if (!last_error_message.empty()) {
        oss << "    \"last_error_message\": \"" << last_error_message << "\",\n";
        auto error_time_t = std::chrono::system_clock::to_time_t(last_error_time);
        oss << "    \"last_error_time\": \"" << std::put_time(std::gmtime(&error_time_t), "%Y-%m-%dT%H:%M:%SZ") << "\"\n";
    } else {
        oss << "    \"last_error_message\": null\n";
    }
    oss << "  }\n";
    oss << "}";
    
    return oss.str();
}

std::string HealthCheckResult::to_string() const {
    std::ostringstream oss;
    
    oss << "Health Status: ";
    switch (status) {
        case HealthStatus::Healthy: oss << "HEALTHY"; break;
        case HealthStatus::Degraded: oss << "DEGRADED"; break;
        case HealthStatus::Unhealthy: oss << "UNHEALTHY"; break;
    }
    oss << "\n";
    
    oss << "Message: " << message << "\n";
    oss << "Messages Logged: " << messages_logged << "\n";
    oss << "Messages Dropped: " << messages_dropped << " (" << (drop_rate * 100.0) << "%)\n";
    oss << "Messages Filtered: " << messages_filtered << "\n";
    oss << "Errors: " << errors << " (" << (error_rate * 100.0) << "%)\n";
    oss << "Throughput: " << messages_per_second << " msg/sec\n";
    oss << "Avg Latency: " << avg_latency_us << " μs\n";
    oss << "Max Latency: " << max_latency_us << " μs\n";
    oss << "Queue Depth: " << queue_depth << "/" << max_queue_depth << "\n";
    
    if (queue_full_warning) {
        oss << "⚠️  Warning: Queue near capacity\n";
    }
    if (high_latency_warning) {
        oss << "⚠️  Warning: High latency detected\n";
    }
    
    return oss.str();
}

HealthChecker::HealthChecker(const HealthCheckConfig& config)
    : config_(config) {}

HealthCheckResult HealthChecker::check_logger(const Logger& logger, const LogMetrics& metrics) const {
    return check_metrics(metrics, 10000);
}

HealthCheckResult HealthChecker::check_metrics(const LogMetrics& metrics, size_t queue_capacity) const {
    HealthCheckResult result;
    result.timestamp = std::chrono::system_clock::now();
    

    auto snapshot = metrics.get_snapshot();
    result.messages_logged = snapshot.messages_logged;
    result.messages_dropped = snapshot.messages_dropped;
    result.messages_filtered = snapshot.messages_filtered;
    result.errors = snapshot.errors;
    result.messages_per_second = snapshot.messages_per_second;
    result.avg_latency_us = snapshot.avg_log_latency_us;
    result.max_latency_us = snapshot.max_log_latency_us;
    result.queue_depth = snapshot.current_queue_depth;
    result.max_queue_depth = snapshot.max_queue_depth;
    

    uint64_t total_attempts = result.messages_logged + result.messages_dropped;
    result.drop_rate = total_attempts > 0 ? static_cast<double>(result.messages_dropped) / total_attempts : 0.0;
    result.error_rate = result.messages_logged > 0 ? static_cast<double>(result.errors) / result.messages_logged : 0.0;
    
  
    double queue_usage = queue_capacity > 0 ? static_cast<double>(result.queue_depth) / queue_capacity : 0.0;
    result.queue_full_warning = queue_usage >= config_.max_queue_usage_degraded;
    result.high_latency_warning = result.max_latency_us >= config_.max_latency_us_degraded;
    

    result.status = determine_status(
        result.drop_rate,
        result.error_rate,
        result.max_latency_us,
        queue_usage
    );
    
    
    std::ostringstream msg;
    switch (result.status) {
        case HealthStatus::Healthy:
            msg << "All systems operational";
            break;
        case HealthStatus::Degraded:
            msg << "Performance degraded: ";
            if (result.drop_rate > config_.max_drop_rate_healthy) {
                msg << "high drop rate (" << (result.drop_rate * 100.0) << "%) ";
            }
            if (result.error_rate > config_.max_error_rate_healthy) {
                msg << "high error rate (" << (result.error_rate * 100.0) << "%) ";
            }
            if (result.max_latency_us > config_.max_latency_us_healthy) {
                msg << "high latency (" << result.max_latency_us << "μs) ";
            }
            if (queue_usage > config_.max_queue_usage_healthy) {
                msg << "queue usage (" << (queue_usage * 100.0) << "%) ";
            }
            break;
        case HealthStatus::Unhealthy:
            msg << "Critical issues detected: ";
            if (result.drop_rate > config_.max_drop_rate_degraded) {
                msg << "critical drop rate (" << (result.drop_rate * 100.0) << "%) ";
            }
            if (result.error_rate > config_.max_error_rate_degraded) {
                msg << "critical error rate (" << (result.error_rate * 100.0) << "%) ";
            }
            if (result.max_latency_us > config_.max_latency_us_degraded) {
                msg << "critical latency (" << result.max_latency_us << "μs) ";
            }
            if (queue_usage > config_.max_queue_usage_degraded) {
                msg << "queue near capacity (" << (queue_usage * 100.0) << "%) ";
            }
            break;
    }
    result.message = msg.str();
    
    return result;
}

void HealthChecker::set_config(const HealthCheckConfig& config) {
    config_ = config;
}

HealthStatus HealthChecker::determine_status(
    double drop_rate,
    double error_rate,
    uint64_t max_latency,
    double queue_usage) const {
    

    if (drop_rate > config_.max_drop_rate_degraded ||
        error_rate > config_.max_error_rate_degraded ||
        max_latency > config_.max_latency_us_degraded ||
        queue_usage > config_.max_queue_usage_degraded) {
        return HealthStatus::Unhealthy;
    }
    
 
    if (drop_rate > config_.max_drop_rate_healthy ||
        error_rate > config_.max_error_rate_healthy ||
        max_latency > config_.max_latency_us_healthy ||
        queue_usage > config_.max_queue_usage_healthy) {
        return HealthStatus::Degraded;
    }
    
    return HealthStatus::Healthy;
}

HealthRegistry& HealthRegistry::instance() {
    static HealthRegistry registry;
    return registry;
}

void HealthRegistry::register_logger(const std::string& name, std::shared_ptr<Logger> logger) {
    std::lock_guard<std::mutex> lock(mutex_);
    LoggerEntry entry;
    entry.logger = logger;
    entry.custom_checker = nullptr;
    loggers_[name] = entry;
}

void HealthRegistry::unregister_logger(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    loggers_.erase(name);
}

HealthCheckResult HealthRegistry::check_logger(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = loggers_.find(name);
    if (it == loggers_.end()) {
        HealthCheckResult result;
        result.status = HealthStatus::Unhealthy;
        result.message = "Logger not found: " + name;
        result.timestamp = std::chrono::system_clock::now();
        return result;
    }
    
    auto logger = it->second.logger.lock();
    if (!logger) {
        HealthCheckResult result;
        result.status = HealthStatus::Unhealthy;
        result.message = "Logger expired: " + name;
        result.timestamp = std::chrono::system_clock::now();
        return result;
    }
    
    const auto& checker = it->second.custom_checker ? it->second.custom_checker : health_checker_;

    LogMetrics metrics;
    HealthCheckResult result = checker->check_logger(*logger, metrics);
    
    result.last_error_message = it->second.last_error_message;
    result.last_error_time = it->second.last_error_time;
    
    return result;
}

std::map<std::string, HealthCheckResult> HealthRegistry::check_all() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::map<std::string, HealthCheckResult> results;
    for (const auto& [name, entry] : loggers_) {
        auto logger = entry.logger.lock();
        if (logger) {
            const auto& checker = entry.custom_checker ? entry.custom_checker : health_checker_;
            LogMetrics metrics;
            HealthCheckResult result = checker->check_logger(*logger, metrics);
            result.last_error_message = entry.last_error_message;
            result.last_error_time = entry.last_error_time;
            results[name] = result;
        }
    }
    
    return results;
}

std::string HealthRegistry::export_json() const {
    auto results = check_all();
    
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"overall_status\": \"";
    
    auto overall = get_overall_status();
    switch (overall) {
        case HealthStatus::Healthy: oss << "healthy"; break;
        case HealthStatus::Degraded: oss << "degraded"; break;
        case HealthStatus::Unhealthy: oss << "unhealthy"; break;
    }
    oss << "\",\n";
    
    oss << "  \"loggers\": [\n";
    bool first = true;
    for (const auto& [name, result] : results) {
        if (!first) oss << ",\n";
        first = false;
        
        oss << "    {\n";
        oss << "      \"name\": \"" << name << "\",\n";
        

        std::string result_json = result.to_json();
        std::istringstream iss(result_json);
        std::string line;
        bool first_line = true;
        while (std::getline(iss, line)) {
            if (!first_line) oss << "\n";
            if (!line.empty()) {
                oss << "      " << line;
            }
            first_line = false;
        }
        oss << "\n    }";
    }
    oss << "\n  ]\n";
    oss << "}";
    
    return oss.str();
}

HealthStatus HealthRegistry::get_overall_status() const {
    auto results = check_all();
    
    if (results.empty()) {
        return HealthStatus::Healthy;
    }
    
    bool has_unhealthy = false;
    bool has_degraded = false;
    
    for (const auto& [name, result] : results) {
        if (result.status == HealthStatus::Unhealthy) {
            has_unhealthy = true;
        } else if (result.status == HealthStatus::Degraded) {
            has_degraded = true;
        }
    }
    
    if (has_unhealthy) {
        return HealthStatus::Unhealthy;
    } else if (has_degraded) {
        return HealthStatus::Degraded;
    } else {
        return HealthStatus::Healthy;
    }
}

void HealthRegistry::set_health_checker(std::shared_ptr<HealthChecker> checker) {
    std::lock_guard<std::mutex> lock(mutex_);
    health_checker_ = std::move(checker);
}

std::atomic<bool> HealthRegistry::auto_registration_enabled_{false};

void HealthRegistry::register_logger(const std::string& name, std::shared_ptr<Logger> logger,
                                     const HealthCheckConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    LoggerEntry entry;
    entry.logger = logger;
    entry.custom_checker = std::make_shared<HealthChecker>(config);
    loggers_[name] = entry;
}

void HealthRegistry::set_logger_config(const std::string& name, const HealthCheckConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
        it->second.custom_checker = std::make_shared<HealthChecker>(config);
    }
}

void HealthRegistry::register_state_change_callback(HealthStateChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_change_callbacks_.push_back(std::move(callback));
}

void HealthRegistry::clear_state_change_callbacks() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_change_callbacks_.clear();
}

void HealthRegistry::record_error(const std::string& logger_name, const std::string& error_message) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loggers_.find(logger_name);
    if (it != loggers_.end()) {
        it->second.last_error_message = error_message;
        it->second.last_error_time = std::chrono::system_clock::now();
    }
}

void HealthRegistry::enable_auto_registration(bool enable) {
    auto_registration_enabled_.store(enable, std::memory_order_release);
}

bool HealthRegistry::is_auto_registration_enabled() {
    return auto_registration_enabled_.load(std::memory_order_acquire);
}

void HealthRegistry::auto_register(const std::string& name, std::shared_ptr<Logger> logger) {
    if (auto_registration_enabled_.load(std::memory_order_acquire)) {
        instance().register_logger(name, logger);
    }
}

void HealthRegistry::notify_state_change(const std::string& name, HealthStatus old_status,
                                         HealthStatus new_status, const HealthCheckResult& result) {
    for (const auto& callback : state_change_callbacks_) {
        callback(name, old_status, new_status, result);
    }
}

AggregateHealthResult HealthRegistry::check_all_aggregate() const {
    auto individual_results = check_all();
    
    AggregateHealthResult agg;
    agg.timestamp = std::chrono::system_clock::now();
    agg.total_loggers = individual_results.size();
    agg.healthy_count = 0;
    agg.degraded_count = 0;
    agg.unhealthy_count = 0;
    agg.total_messages_logged = 0;
    agg.total_messages_dropped = 0;
    agg.total_errors = 0;
    agg.avg_messages_per_second = 0.0;
    agg.worst_logger_status = HealthStatus::Healthy;
    agg.individual_results = individual_results;
    
    for (const auto& [name, result] : individual_results) {
        agg.total_messages_logged += result.messages_logged;
        agg.total_messages_dropped += result.messages_dropped;
        agg.total_errors += result.errors;
        agg.avg_messages_per_second += result.messages_per_second;
        
        switch (result.status) {
            case HealthStatus::Healthy:
                agg.healthy_count++;
                break;
            case HealthStatus::Degraded:
                agg.degraded_count++;
                if (agg.worst_logger_status == HealthStatus::Healthy) {
                    agg.worst_logger_name = name;
                    agg.worst_logger_status = HealthStatus::Degraded;
                }
                break;
            case HealthStatus::Unhealthy:
                agg.unhealthy_count++;
                if (agg.worst_logger_status != HealthStatus::Unhealthy) {
                    agg.worst_logger_name = name;
                    agg.worst_logger_status = HealthStatus::Unhealthy;
                }
                break;
        }
    }
    
    if (agg.unhealthy_count > 0) {
        agg.overall_status = HealthStatus::Unhealthy;
    } else if (agg.degraded_count > 0) {
        agg.overall_status = HealthStatus::Degraded;
    } else {
        agg.overall_status = HealthStatus::Healthy;
    }
    
    return agg;
}

std::string AggregateHealthResult::to_json() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "{\n";
    oss << "  \"overall_status\": \"";
    switch (overall_status) {
        case HealthStatus::Healthy: oss << "healthy"; break;
        case HealthStatus::Degraded: oss << "degraded"; break;
        case HealthStatus::Unhealthy: oss << "unhealthy"; break;
    }
    oss << "\",\n";
    
    auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
    oss << "  \"timestamp\": \"" << std::put_time(std::gmtime(&time_t_val), "%Y-%m-%dT%H:%M:%SZ") << "\",\n";
    
    oss << "  \"summary\": {\n";
    oss << "    \"total_loggers\": " << total_loggers << ",\n";
    oss << "    \"healthy\": " << healthy_count << ",\n";
    oss << "    \"degraded\": " << degraded_count << ",\n";
    oss << "    \"unhealthy\": " << unhealthy_count << "\n";
    oss << "  },\n";
    
    oss << "  \"aggregate_metrics\": {\n";
    oss << "    \"total_messages_logged\": " << total_messages_logged << ",\n";
    oss << "    \"total_messages_dropped\": " << total_messages_dropped << ",\n";
    oss << "    \"total_errors\": " << total_errors << ",\n";
    oss << "    \"avg_messages_per_second\": " << avg_messages_per_second << "\n";
    oss << "  },\n";
    
    if (!worst_logger_name.empty() && worst_logger_status != HealthStatus::Healthy) {
        oss << "  \"worst_logger\": {\n";
        oss << "    \"name\": \"" << worst_logger_name << "\",\n";
        oss << "    \"status\": \"";
        switch (worst_logger_status) {
            case HealthStatus::Degraded: oss << "degraded"; break;
            case HealthStatus::Unhealthy: oss << "unhealthy"; break;
            default: oss << "healthy"; break;
        }
        oss << "\"\n";
        oss << "  },\n";
    }
    
    oss << "  \"loggers\": " << individual_results.size() << "\n";
    oss << "}";
    
    return oss.str();
}

std::string AggregateHealthResult::to_string() const {
    std::ostringstream oss;
    
    oss << "=== Aggregate Health Check ===\n";
    oss << "Overall Status: ";
    switch (overall_status) {
        case HealthStatus::Healthy: oss << "HEALTHY"; break;
        case HealthStatus::Degraded: oss << "DEGRADED"; break;
        case HealthStatus::Unhealthy: oss << "UNHEALTHY"; break;
    }
    oss << "\n\n";
    
    oss << "Loggers: " << total_loggers << " total\n";
    oss << "  - Healthy: " << healthy_count << "\n";
    oss << "  - Degraded: " << degraded_count << "\n";
    oss << "  - Unhealthy: " << unhealthy_count << "\n\n";
    
    oss << "Aggregate Metrics:\n";
    oss << "  - Total Messages: " << total_messages_logged << "\n";
    oss << "  - Total Dropped: " << total_messages_dropped << "\n";
    oss << "  - Total Errors: " << total_errors << "\n";
    oss << "  - Throughput: " << avg_messages_per_second << " msg/sec\n";
    
    if (!worst_logger_name.empty() && worst_logger_status != HealthStatus::Healthy) {
        oss << "\nWorst Performing Logger: " << worst_logger_name << " (";
        switch (worst_logger_status) {
            case HealthStatus::Degraded: oss << "DEGRADED"; break;
            case HealthStatus::Unhealthy: oss << "UNHEALTHY"; break;
            default: break;
        }
        oss << ")\n";
    }
    
    return oss.str();
}

} // namespace Zyrnix
