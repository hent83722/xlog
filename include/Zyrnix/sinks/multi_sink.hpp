#pragma once
#include "Zyrnix/log_sink.hpp"
#include <vector>
#include <memory>


namespace Zyrnix {

class MultiSink : public LogSink {
public:
    MultiSink() = default;

    void add_sink(LogSinkPtr sink) {
        sinks.push_back(sink);
    }

    void log(const std::string& logger_name, LogLevel level, const std::string& message) override {
        for (auto& sink : sinks) {
            sink->log(logger_name, level, message);
        }
    }

private:
    std::vector<LogSinkPtr> sinks;
};

}
