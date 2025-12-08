#pragma once

#include <string>
#include <memory>
#include "logger.hpp"
#include "task_manager.hpp"
#include "user_service.hpp"

namespace taskapp {

class ApiHandler {
public:
    ApiHandler(std::shared_ptr<xlog::Logger> logger,
              std::shared_ptr<TaskManager> task_mgr,
              std::shared_ptr<UserService> user_svc);
    
    // Simulate API endpoints
    void handle_create_task(const std::string& user, const std::string& title,
                           const std::string& priority);
    
    void handle_list_tasks(const std::string& user);
    
    void handle_update_task(const std::string& user, int task_id,
                          const std::string& new_status);
    
    void handle_login(const std::string& username, const std::string& password);
    
    void handle_upgrade_account(const std::string& username);

private:
    std::shared_ptr<xlog::Logger> logger_;
    std::shared_ptr<TaskManager> task_manager_;
    std::shared_ptr<UserService> user_service_;
    
    std::string generate_request_id();
    int request_counter_ = 0;
};

} // namespace taskapp
