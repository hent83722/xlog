#pragma once
#include <string>
#include "xlog/log_level.hpp"

namespace xlog {

struct LogMessage {
    LogLevel level;
    std::string text;
};

}
