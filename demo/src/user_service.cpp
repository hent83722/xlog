#include "user_service.hpp"
#include "log_macros.hpp"
#include "log_context.hpp"

namespace taskapp {

UserService::UserService(std::shared_ptr<xlog::Logger> logger)
    : logger_(logger) {
    XLOG_INFO(logger_, "UserService initialized");
}

bool UserService::register_user(const std::string& username, const std::string& email,
                                UserRole role, bool is_premium) {
    xlog::ScopedContext ctx({
        {"username", username},
        {"email", email},
        {"action", "register"}
    });
    
    if (users_.find(username) != users_.end()) {
        XLOG_WARN(logger_, "Registration failed: username already exists");
        return false;
    }
    
    users_.emplace(username, User(username, email, role, is_premium));
    
    XLOG_INFO(logger_, "User registered successfully (premium: {})", is_premium);
    
    // Debug log for admin users
    XLOG_DEBUG_IF(logger_, role == UserRole::Admin,
                  "Admin user registered: {}", username);
    
    return true;
}

User* UserService::authenticate(const std::string& username, const std::string& password) {
    xlog::ScopedContext ctx({
        {"username", username},
        {"action", "authenticate"}
    });
    
    XLOG_DEBUG(logger_, "Authentication attempt");
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        XLOG_WARN(logger_, "Authentication failed: user not found");
        return nullptr;
    }
    
    // Simulate password check (always succeed for demo)
    active_sessions_[username] = true;
    
    XLOG_INFO(logger_, "Authentication successful (role: {}, premium: {})",
              static_cast<int>(it->second.role), it->second.is_premium);
    
    return &it->second;
}

bool UserService::upgrade_to_premium(const std::string& username) {
    xlog::ScopedContext ctx({
        {"username", username},
        {"action", "upgrade_premium"}
    });
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        XLOG_ERROR(logger_, "Upgrade failed: user not found");
        return false;
    }
    
    if (it->second.is_premium) {
        XLOG_WARN(logger_, "User is already premium");
        return false;
    }
    
    it->second.is_premium = true;
    XLOG_INFO(logger_, "User upgraded to premium successfully");
    
    return true;
}

void UserService::logout(const std::string& username) {
    xlog::ScopedContext ctx({
        {"username", username},
        {"action", "logout"}
    });
    
    active_sessions_.erase(username);
    XLOG_INFO(logger_, "User logged out");
}

bool UserService::has_permission(const std::string& username, const std::string& action) {
    xlog::ScopedContext ctx({
        {"username", username},
        {"action", action}
    });
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        XLOG_WARN(logger_, "Permission check failed: user not found");
        return false;
    }
    
    bool has_access = it->second.role == UserRole::Admin || 
                     (it->second.role == UserRole::User && it->second.is_premium);
    
    XLOG_DEBUG(logger_, "Permission check: {} (granted: {})", action, has_access);
    
    return has_access;
}

} // namespace taskapp
