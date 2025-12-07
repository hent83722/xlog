#pragma once
#include <memory>
#include <string>
#include "log_level.hpp"
#include "formatter.hpp"

namespace xlog {

class LogSink {
public:
    virtual ~LogSink() = default;
    virtual void log(const std::string& name, LogLevel level, const std::string& message) = 0;

    void set_level(LogLevel lvl) { level = lvl; }
    LogLevel get_level() const { return level; }

protected:
    LogLevel level = LogLevel::Trace;
    Formatter formatter;
};

using LogSinkPtr = std::shared_ptr<LogSink>;

}
