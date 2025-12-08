#include "task_manager.hpp"
#include "log_macros.hpp"
#include "log_context.hpp"
#include <algorithm>

namespace taskapp {

TaskManager::TaskManager(std::shared_ptr<xlog::Logger> logger)
    : logger_(logger) {
    XLOG_INFO(logger_, "TaskManager initialized");
}

int TaskManager::create_task(const std::string& title, const std::string& description,
                             const std::string& priority, const std::string& user) {
    int task_id = next_id_++;
    
    // Use scoped context to add task metadata to all logs in this scope
    xlog::ScopedContext task_ctx({
        {"task_id", std::to_string(task_id)},
        {"priority", priority},
        {"assigned_to", user}
    });
    
    XLOG_DEBUG_IF(logger_, priority == "critical",
                  "Creating CRITICAL priority task: {}", title);
    
    tasks_.emplace_back(task_id, title, description, priority, user);
    
    XLOG_INFO(logger_, "Task created: '{}'", title);
    
    // Log warning for high priority tasks
    XLOG_WARN_IF(logger_, priority == "high" || priority == "critical",
                 "High priority task assigned - immediate attention required");
    
    return task_id;
}

bool TaskManager::update_task_status(int task_id, const std::string& new_status,
                                     const std::string& user) {
    xlog::ScopedContext ctx({
        {"task_id", std::to_string(task_id)},
        {"user", user},
        {"action", "update_status"}
    });
    
    Task* task = find_task(task_id);
    if (!task) {
        XLOG_ERROR(logger_, "Task not found: {}", task_id);
        return false;
    }
    
    std::string old_status = task->status;
    task->status = new_status;
    
    XLOG_INFO(logger_, "Task status updated: {} -> {}", old_status, new_status);
    
    // Log completion with context
    if (new_status == "completed") {
        XLOG_INFO(logger_, "Task '{}' completed by {}", task->title, user);
    }
    
    return true;
}

std::vector<Task> TaskManager::get_user_tasks(const std::string& user) {
    XLOG_DEBUG(logger_, "Fetching tasks for user: {}", user);
    
    std::vector<Task> user_tasks;
    std::copy_if(tasks_.begin(), tasks_.end(), std::back_inserter(user_tasks),
                [&user](const Task& t) { return t.assigned_to == user; });
    
    XLOG_DEBUG(logger_, "Found {} tasks for user: {}", user_tasks.size(), user);
    return user_tasks;
}

std::vector<Task> TaskManager::get_high_priority_tasks() {
    XLOG_TRACE(logger_, "Querying high priority tasks");
    
    std::vector<Task> high_priority;
    std::copy_if(tasks_.begin(), tasks_.end(), std::back_inserter(high_priority),
                [](const Task& t) { 
                    return t.priority == "high" || t.priority == "critical";
                });
    
    XLOG_INFO_IF(logger_, high_priority.size() > 10,
                 "Warning: {} high priority tasks pending!", high_priority.size());
    
    return high_priority;
}

void TaskManager::cleanup_completed_tasks() {
    XLOG_INFO(logger_, "Starting cleanup of completed tasks");
    
    size_t before = tasks_.size();
    tasks_.erase(
        std::remove_if(tasks_.begin(), tasks_.end(),
                      [](const Task& t) { return t.status == "completed"; }),
        tasks_.end()
    );
    size_t after = tasks_.size();
    
    XLOG_INFO(logger_, "Cleanup complete: removed {} completed tasks", before - after);
}

void TaskManager::print_statistics() {
    size_t pending = std::count_if(tasks_.begin(), tasks_.end(),
                                   [](const Task& t) { return t.status == "pending"; });
    size_t in_progress = std::count_if(tasks_.begin(), tasks_.end(),
                                       [](const Task& t) { return t.status == "in_progress"; });
    size_t completed = std::count_if(tasks_.begin(), tasks_.end(),
                                     [](const Task& t) { return t.status == "completed"; });
    
    XLOG_INFO(logger_, "=== Task Statistics ===");
    XLOG_INFO(logger_, "Total tasks: {}", tasks_.size());
    XLOG_INFO(logger_, "Pending: {}, In Progress: {}, Completed: {}", 
              pending, in_progress, completed);
}

Task* TaskManager::find_task(int task_id) {
    auto it = std::find_if(tasks_.begin(), tasks_.end(),
                          [task_id](const Task& t) { return t.id == task_id; });
    return it != tasks_.end() ? &(*it) : nullptr;
}

} // namespace taskapp
