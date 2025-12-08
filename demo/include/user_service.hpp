#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "logger.hpp"

namespace taskapp {

enum class UserRole {
    Guest,
    User,
    Admin
};

struct User {
    std::string username;
    std::string email;
    UserRole role;
    bool is_premium;
    
    User(const std::string& name, const std::string& mail, 
         UserRole r, bool premium = false)
        : username(name), email(mail), role(r), is_premium(premium) {}
};

class UserService {
public:
    explicit UserService(std::shared_ptr<xlog::Logger> logger);
    
    // User management
    bool register_user(const std::string& username, const std::string& email,
                      UserRole role = UserRole::User, bool is_premium = false);
    
    User* authenticate(const std::string& username, const std::string& password);
    
    bool upgrade_to_premium(const std::string& username);
    
    void logout(const std::string& username);
    
    // Access control
    bool has_permission(const std::string& username, const std::string& action);

private:
    std::shared_ptr<xlog::Logger> logger_;
    std::unordered_map<std::string, User> users_;
    std::unordered_map<std::string, bool> active_sessions_;
};

} // namespace taskapp
