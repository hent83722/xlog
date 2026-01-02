#pragma once
#include <memory>
#include <string>
#include "log_level.hpp"
#include "formatter.hpp"

namespace Zyrnix {

class LogSink {
public:
    virtual ~LogSink() = default;
    virtual void log(const std::string& name, LogLevel level, const std::string& message) = 0;

    // Cloud-aware sinks (v1.1.3)
    // Override in cloud sinks (e.g., Loki, CloudWatch, Azure) to enable
    // per-sink redaction routing and health reporting.
    virtual bool is_cloud_sink() const { return false; }

    void set_level(LogLevel lvl) { level = lvl; }
    LogLevel get_level() const { return level; }

protected:
    LogLevel level = LogLevel::Trace;
    Formatter formatter;
};

using LogSinkPtr = std::shared_ptr<LogSink>;

}
