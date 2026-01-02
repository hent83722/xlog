#include "Zyrnix/log_context.hpp"
#include <algorithm>

namespace Zyrnix {

thread_local LogContext::ContextMap LogContext::context_;

void LogContext::set(const std::string& key, const std::string& value) {
    context_[key] = value;
}

std::string LogContext::get(const std::string& key) {
    auto it = context_.find(key);
    if (it != context_.end()) {
        return it->second;
    }
    return "";
}

void LogContext::remove(const std::string& key) {
    context_.erase(key);
}

void LogContext::clear() {
    context_.clear();
}

LogContext::ContextMap LogContext::get_all() {
    return context_;
}

bool LogContext::contains(const std::string& key) {
    return context_.find(key) != context_.end();
}

ScopedContext::ScopedContext() = default;

ScopedContext::ScopedContext(const LogContext::ContextMap& initial_context) {
    for (const auto& [key, value] : initial_context) {
        set(key, value);
    }
}

ScopedContext::~ScopedContext() {
    for (const auto& key : scoped_keys_) {
        LogContext::remove(key);
    }
}

ScopedContext& ScopedContext::set(const std::string& key, const std::string& value) {
    if (!LogContext::contains(key)) {
        scoped_keys_.push_back(key);
    }
    LogContext::set(key, value);
    return *this;
}

ScopedContext& ScopedContext::remove(const std::string& key) {
    LogContext::remove(key);
    auto it = std::find(scoped_keys_.begin(), scoped_keys_.end(), key);
    if (it != scoped_keys_.end()) {
        scoped_keys_.erase(it);
    }
    return *this;
}

std::string ScopedContext::get(const std::string& key) const {
    return LogContext::get(key);
}

LogContext::ContextMap ScopedContext::get_all() const {
    return LogContext::get_all();
}

} // namespace Zyrnix
