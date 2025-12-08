#pragma once

#include <string>
#include <vector>
#include <memory>
#include "logger.hpp"

namespace taskapp {

struct Task {
    int id;
    std::string title;
    std::string description;
    std::string priority;  // "low", "medium", "high", "critical"
    std::string status;    // "pending", "in_progress", "completed"
    std::string assigned_to;
    
    Task(int id, const std::string& title, const std::string& desc, 
         const std::string& priority, const std::string& user)
        : id(id), title(title), description(desc), priority(priority),
          status("pending"), assigned_to(user) {}
};

class TaskManager {
public:
    explicit TaskManager(std::shared_ptr<xlog::Logger> logger);
    
    // Task operations
    int create_task(const std::string& title, const std::string& description,
                   const std::string& priority, const std::string& user);
    
    bool update_task_status(int task_id, const std::string& new_status,
                          const std::string& user);
    
    std::vector<Task> get_user_tasks(const std::string& user);
    
    std::vector<Task> get_high_priority_tasks();
    
    void cleanup_completed_tasks();
    
    // Statistics
    void print_statistics();

private:
    std::shared_ptr<xlog::Logger> logger_;
    std::vector<Task> tasks_;
    int next_id_ = 1;
    
    Task* find_task(int task_id);
};

} // namespace taskapp
