#pragma once
#include <string>
#include "logger.hpp"
#include "config.hpp"

namespace xlog {

LoggerPtr create_logger(const std::string& name, const Config& cfg = Config());

}
#define LOG_TRACE(logger, msg) logger->trace(msg)
#define LOG_DEBUG(logger, msg) logger->debug(msg)
#define LOG_INFO(logger, msg) logger->info(msg)
#define LOG_WARN(logger, msg) logger->warn(msg)
#define LOG_ERROR(logger, msg) logger->error(msg)
#define LOG_CRITICAL(logger, msg) logger->critical(msg)
