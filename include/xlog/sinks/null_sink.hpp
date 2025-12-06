#pragma once
#include "log_sink.hpp"

namespace xlog {

class NullSink : public LogSink {
public:
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override {}
};

}
