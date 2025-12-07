#pragma once
#include "../log_sink.hpp"
#include "../formatter.hpp"

namespace xlog {

class StdoutSink : public LogSink {
public:
    StdoutSink();
    void log(const std::string& name, LogLevel level, const std::string& message) override;

private:
};

}
