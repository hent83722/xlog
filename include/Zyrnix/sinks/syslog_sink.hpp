#pragma once
#include "../log_sink.hpp"
#include "../log_level.hpp"
#include <syslog.h>
#include <string>
#include <mutex>

namespace Zyrnix {

class SyslogSink : public LogSink {
public:
    explicit SyslogSink(const std::string& ident = "", int option = LOG_PID, int facility = LOG_USER);
    ~SyslogSink();
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;

private:
    std::string ident;
    int option;
    int facility;
    std::mutex mtx;
    int map_level(LogLevel lvl);
};

}
