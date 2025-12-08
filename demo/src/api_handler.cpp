#include "api_handler.hpp"
#include "log_macros.hpp"
#include "log_context.hpp"
#include <sstream>
#include <iomanip>

namespace taskapp {

ApiHandler::ApiHandler(std::shared_ptr<xlog::Logger> logger,
                      std::shared_ptr<TaskManager> task_mgr,
                      std::shared_ptr<UserService> user_svc)
    : logger_(logger), task_manager_(task_mgr), user_service_(user_svc) {
    XLOG_INFO(logger_, "ApiHandler initialized");
}

std::string ApiHandler::generate_request_id() {
    std::ostringstream oss;
    oss << "req-" << std::setfill('0') << std::setw(6) << ++request_counter_;
    return oss.str();
}

void ApiHandler::handle_create_task(const std::string& user, const std::string& title,
                                    const std::string& priority) {
    std::string req_id = generate_request_id();
    
    // Add request context that will appear in all logs during this request
    xlog::ScopedContext request_ctx({
        {"request_id", req_id},
        {"endpoint", "POST /tasks"},
        {"user", user}
    });
    
    XLOG_INFO(logger_, "API Request: Create task");
    XLOG_DEBUG(logger_, "Task details - title: '{}', priority: {}", title, priority);
    
    // Check permissions
    if (!user_service_->has_permission(user, "create_task")) {
        XLOG_WARN(logger_, "Permission denied for user");
        return;
    }
    
    // Create the task
    int task_id = task_manager_->create_task(title, "Sample description", priority, user);
    
    XLOG_INFO(logger_, "Task created successfully with ID: {}", task_id);
    
    // Log critical priority tasks specially
    XLOG_CRITICAL_IF(logger_, priority == "critical",
                     "CRITICAL TASK CREATED - Immediate escalation required!");
}

void ApiHandler::handle_list_tasks(const std::string& user) {
    std::string req_id = generate_request_id();
    
    xlog::ScopedContext request_ctx({
        {"request_id", req_id},
        {"endpoint", "GET /tasks"},
        {"user", user}
    });
    
    XLOG_INFO(logger_, "API Request: List tasks");
    
    auto tasks = task_manager_->get_user_tasks(user);
    
    XLOG_INFO(logger_, "Returning {} tasks", tasks.size());
    
    // Trace log for debugging (only in debug builds)
    XLOG_TRACE(logger_, "Task list request completed successfully");
}

void ApiHandler::handle_update_task(const std::string& user, int task_id,
                                   const std::string& new_status) {
    std::string req_id = generate_request_id();
    
    xlog::ScopedContext request_ctx({
        {"request_id", req_id},
        {"endpoint", "PUT /tasks"},
        {"user", user},
        {"task_id", std::to_string(task_id)}
    });
    
    XLOG_INFO(logger_, "API Request: Update task status to '{}'", new_status);
    
    bool success = task_manager_->update_task_status(task_id, new_status, user);
    
    if (success) {
        XLOG_INFO(logger_, "Task update successful");
    } else {
        XLOG_ERROR(logger_, "Task update failed");
    }
}

void ApiHandler::handle_login(const std::string& username, const std::string& password) {
    std::string req_id = generate_request_id();
    
    xlog::ScopedContext request_ctx({
        {"request_id", req_id},
        {"endpoint", "POST /auth/login"},
        {"username", username}
    });
    
    XLOG_INFO(logger_, "API Request: User login");
    
    User* user = user_service_->authenticate(username, password);
    
    if (user) {
        XLOG_INFO(logger_, "Login successful");
        
        // Add user type to context for subsequent operations
        xlog::LogContext::add_field("user_type", user->is_premium ? "premium" : "standard");
    } else {
        XLOG_WARN(logger_, "Login failed");
    }
}

void ApiHandler::handle_upgrade_account(const std::string& username) {
    std::string req_id = generate_request_id();
    
    xlog::ScopedContext request_ctx({
        {"request_id", req_id},
        {"endpoint", "POST /users/upgrade"},
        {"username", username}
    });
    
    XLOG_INFO(logger_, "API Request: Upgrade to premium");
    
    bool success = user_service_->upgrade_to_premium(username);
    
    if (success) {
        XLOG_INFO(logger_, "Account upgrade successful - premium features enabled");
    } else {
        XLOG_WARN(logger_, "Account upgrade failed");
    }
}

} // namespace taskapp
