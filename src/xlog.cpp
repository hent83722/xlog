#include "xlog/xlog.hpp"
#include "xlog/logger.hpp"
#include "xlog/config.hpp"
#include "xlog/async/async_logger.hpp"

namespace xlog {

LoggerPtr create_logger(const std::string& name, const Config&) {
    return std::make_shared<Logger>(name);
}

AsyncLoggerPtr create_async_logger(LoggerPtr logger, const Config&) {
    return std::make_shared<AsyncLogger>(logger);
}

}
