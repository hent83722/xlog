#pragma once
#include "Zyrnix_features.hpp"
#include "log_level.hpp"
#include "log_metrics.hpp"
#include <string>
#include <memory>
#include <chrono>
#include <map>
#include <functional>
#include <vector>

namespace Zyrnix {

class Logger;


enum class HealthStatus {
    Healthy,    
    Degraded,     
    Unhealthy     
};


struct HealthCheckResult {
    HealthStatus status;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    
  
    uint64_t messages_logged;
    uint64_t messages_dropped;
    uint64_t messages_filtered;
    uint64_t errors;
    double messages_per_second;
    double avg_latency_us;
    uint64_t max_latency_us;
    size_t queue_depth;
    size_t max_queue_depth;
    

    double drop_rate;         
    double error_rate;         
    bool queue_full_warning;   
    bool high_latency_warning;
    
    std::string last_error_message;
    std::chrono::system_clock::time_point last_error_time;
    
  
    std::string to_json() const;
    
    
    std::string to_string() const;
};


struct HealthCheckConfig {
   
    double max_drop_rate_healthy = 0.01; // 1% drop rate = healthy
    double max_drop_rate_degraded = 0.05;   // 5% drop rate = degraded
    double max_error_rate_healthy = 0.001; // 0.1% error rate = healthy
    double max_error_rate_degraded = 0.01; // 1% error rate = degraded
    uint64_t max_latency_us_healthy = 10000; // 10ms = healthy
    uint64_t max_latency_us_degraded = 50000; // 50ms = degraded
    double max_queue_usage_healthy = 0.7; // 70% queue usage = healthy
    double max_queue_usage_degraded = 0.9;  // 90% queue usage = degraded
};

struct AggregateHealthResult {
    HealthStatus overall_status;
    std::chrono::system_clock::time_point timestamp;
    
    size_t total_loggers;
    size_t healthy_count;
    size_t degraded_count;
    size_t unhealthy_count;
    
    uint64_t total_messages_logged;
    uint64_t total_messages_dropped;
    uint64_t total_errors;
    double avg_messages_per_second;
    
    std::string worst_logger_name;
    HealthStatus worst_logger_status;
    
    std::map<std::string, HealthCheckResult> individual_results;
    
    std::string to_json() const;
    std::string to_string() const;
};

using HealthStateChangeCallback = std::function<void(
    const std::string& logger_name,
    HealthStatus old_status,
    HealthStatus new_status,
    const HealthCheckResult& result
)>;


class HealthChecker {
public:
    explicit HealthChecker(const HealthCheckConfig& config = HealthCheckConfig{});
    

    HealthCheckResult check_logger(const Logger& logger, const LogMetrics& metrics) const;
    
    
    HealthCheckResult check_metrics(const LogMetrics& metrics, size_t queue_capacity = 10000) const;
    
    
    void set_config(const HealthCheckConfig& config);
    HealthCheckConfig get_config() const { return config_; }
    

    static bool is_healthy(const HealthCheckResult& result) { 
        return result.status == HealthStatus::Healthy; 
    }
    
    static bool is_degraded(const HealthCheckResult& result) { 
        return result.status == HealthStatus::Degraded; 
    }
    
    static bool is_unhealthy(const HealthCheckResult& result) { 
        return result.status == HealthStatus::Unhealthy; 
    }
    
private:
    HealthStatus determine_status(
        double drop_rate,
        double error_rate,
        uint64_t max_latency,
        double queue_usage
    ) const;
    
    HealthCheckConfig config_;
};


class HealthRegistry {
public:
    static HealthRegistry& instance();
    

    void register_logger(const std::string& name, std::shared_ptr<Logger> logger);
    
    void register_logger(const std::string& name, std::shared_ptr<Logger> logger,
                        const HealthCheckConfig& config);
    

    void unregister_logger(const std::string& name);
    

    HealthCheckResult check_logger(const std::string& name) const;

    std::map<std::string, HealthCheckResult> check_all() const;
    
    AggregateHealthResult check_all_aggregate() const;
    

    std::string export_json() const;
    
 
    HealthStatus get_overall_status() const;
    
  
    void set_health_checker(std::shared_ptr<HealthChecker> checker);
    
    void set_logger_config(const std::string& name, const HealthCheckConfig& config);
    
    void register_state_change_callback(HealthStateChangeCallback callback);
    void clear_state_change_callbacks();
    
    void record_error(const std::string& logger_name, const std::string& error_message);
    
    static void enable_auto_registration(bool enable = true);
    static bool is_auto_registration_enabled();
    
    static void auto_register(const std::string& name, std::shared_ptr<Logger> logger);
    
private:
    HealthRegistry() : health_checker_(std::make_shared<HealthChecker>()) {}
    
    struct LoggerEntry {
        std::weak_ptr<Logger> logger;
        std::shared_ptr<HealthChecker> custom_checker;
        HealthStatus last_status = HealthStatus::Healthy;
        std::string last_error_message;
        std::chrono::system_clock::time_point last_error_time;
    };
    
    std::map<std::string, LoggerEntry> loggers_;
    std::shared_ptr<HealthChecker> health_checker_;
    std::vector<HealthStateChangeCallback> state_change_callbacks_;
    mutable std::mutex mutex_;
    
    static std::atomic<bool> auto_registration_enabled_;
    
    void notify_state_change(const std::string& name, HealthStatus old_status,
                            HealthStatus new_status, const HealthCheckResult& result);
};


inline std::string handle_health_check_request(const std::string& logger_name = "") {
    auto& registry = HealthRegistry::instance();
    
    if (logger_name.empty()) {
      
        return registry.export_json();
    } else {
   
        auto result = registry.check_logger(logger_name);
        return result.to_json();
    }
}

inline AggregateHealthResult handle_aggregate_health_check() {
    return HealthRegistry::instance().check_all_aggregate();
}

} // namespace Zyrnix
