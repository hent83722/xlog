#include "xlog/sinks/stdout_sink.hpp"
#include "xlog/formatter.hpp"
#include "xlog/color.hpp"
#include "xlog/log_level.hpp"
#include <iostream>

namespace xlog {

StdoutSink::StdoutSink() : formatter(std::make_shared<Formatter>()) {}

void StdoutSink::log(const std::string& name, LogLevel level, const std::string& msg) {
    std::string out = formatter->format(name, level, msg);
    if (level == LogLevel::Error || level == LogLevel::Critical)
        out = apply_color(out, Color::Red);
    else if (level == LogLevel::Warn)
        out = apply_color(out, Color::Yellow);

    std::cout << out << std::endl;
}

}
