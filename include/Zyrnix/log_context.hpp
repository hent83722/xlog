#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>

namespace Zyrnix {

class Logger;

class LogContext {
public:
    using ContextMap = std::unordered_map<std::string, std::string>;

    static void set(const std::string& key, const std::string& value);
    static std::string get(const std::string& key);
    static void remove(const std::string& key);
    static void clear();
    static ContextMap get_all();
    static bool contains(const std::string& key);

private:
    static thread_local ContextMap context_;
};

class ScopedContext {
public:
    ScopedContext();
    explicit ScopedContext(const LogContext::ContextMap& initial_context);
    ~ScopedContext();

    ScopedContext(const ScopedContext&) = delete;
    ScopedContext& operator=(const ScopedContext&) = delete;
    ScopedContext(ScopedContext&&) = delete;
    ScopedContext& operator=(ScopedContext&&) = delete;

    ScopedContext& set(const std::string& key, const std::string& value);
    ScopedContext& remove(const std::string& key);
    std::string get(const std::string& key) const;
    LogContext::ContextMap get_all() const;

private:
    std::vector<std::string> scoped_keys_;
};

} // namespace Zyrnix
