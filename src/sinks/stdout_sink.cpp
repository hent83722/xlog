#include "Zyrnix/sinks/stdout_sink.hpp"
#include "Zyrnix/formatter.hpp"
#include "Zyrnix/color.hpp"
#include "Zyrnix/log_level.hpp"
#include <iostream>

namespace Zyrnix {

StdoutSink::StdoutSink() {}

void StdoutSink::log(const std::string& name, LogLevel level, const std::string& msg) {
    std::string out = formatter.format(name, level, msg);
    if (level == LogLevel::Error || level == LogLevel::Critical)
        out = apply_color(out, Color::Red);
    else if (level == LogLevel::Warn)
        out = apply_color(out, Color::Yellow);

    std::cout << out << std::endl;
}

}
