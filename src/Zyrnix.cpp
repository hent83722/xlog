#include "Zyrnix/Zyrnix.hpp"
#include "Zyrnix/logger.hpp"
#include "Zyrnix/config.hpp"
#include "Zyrnix/async/async_logger.hpp"

namespace Zyrnix {

LoggerPtr create_logger(const std::string& name, const Config&) {
    return std::make_shared<Logger>(name);
}

AsyncLoggerPtr create_async_logger(LoggerPtr logger, const Config&) {
    return std::make_shared<AsyncLogger>(logger);
}

}
