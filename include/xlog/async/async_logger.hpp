#pragma once
#include <memory>
#include "../logger.hpp"

namespace xlog {

class AsyncLogger {
public:
    AsyncLogger(LoggerPtr logger) : logger(logger) {}

    void info(const std::string& msg) { logger->info(msg); }
    void debug(const std::string& msg) { logger->debug(msg); }
    void error(const std::string& msg) { logger->error(msg); }
    void warn(const std::string& msg) { logger->warn(msg); }
    void trace(const std::string& msg) { logger->trace(msg); }
    void critical(const std::string& msg) { logger->critical(msg); }

private:
    LoggerPtr logger;
};

using AsyncLoggerPtr = std::shared_ptr<AsyncLogger>;

}
