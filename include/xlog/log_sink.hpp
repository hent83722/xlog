#pragma once
#include <memory>
#include <string>
#include "log_level.hpp"

namespace xlog {

class LogSink {
public:
    virtual ~LogSink() = default;
    virtual void log(const std::string& name, LogLevel level, const std::string& message) = 0;
};

using LogSinkPtr = std::shared_ptr<LogSink>;

}
