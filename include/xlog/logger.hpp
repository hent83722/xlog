#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "log_sink.hpp"

namespace xlog {

class Logger {
public:
    explicit Logger(std::string name);

    void add_sink(LogSinkPtr sink);
    void log(LogLevel level, const std::string& message);

    void trace(const std::string& msg);
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void critical(const std::string& msg);

private:
    std::string name;
    std::vector<LogSinkPtr> sinks;
    std::mutex mtx;
};

using LoggerPtr = std::shared_ptr<Logger>;

}
