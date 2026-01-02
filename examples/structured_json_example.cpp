#include <Zyrnix/structured_logger.hpp>
#include <map>

int main() {

    auto slog = Zyrnix::StructuredLogger::create("api_server", "api.jsonl");
    

    slog->set_context("service", "user-api");
    slog->set_context("environment", "production");
    slog->set_context("version", "1.0.0");
    

    slog->info("Service started", {
        {"port", "8080"},
        {"workers", "4"}
    });
    

    slog->set_context("request_id", "req-12345");
    slog->info("User login attempt", {
        {"user_id", "user-456"},
        {"ip_address", "192.168.1.100"},
        {"user_agent", "Mozilla/5.0"}
    });
    
    slog->info("User login successful", {
        {"user_id", "user-456"},
        {"duration_ms", "145"}
    });
    

    slog->set_context("request_id", "req-12346");
    slog->warn("High memory usage detected", {
        {"memory_mb", "2048"},
        {"threshold_mb", "1800"}
    });
    

    slog->error("Database connection failed", {
        {"database", "postgres"},
        {"host", "db.example.com"},
        {"error_code", "ECONNREFUSED"}
    });
    

    slog->clear_context();
    slog->info("Service shutdown", {
        {"reason", "maintenance"},
        {"duration_minutes", "30"}
    });
    
    return 0;
}
