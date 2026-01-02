#pragma once
#include <string>
#include "Zyrnix/log_level.hpp"

namespace Zyrnix {

struct LogMessage {
    LogLevel level;
    std::string text;
};

}
