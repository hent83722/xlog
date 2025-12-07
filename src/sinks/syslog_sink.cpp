#include "xlog/sinks/syslog_sink.hpp"
#include <syslog.h>

namespace xlog {

SyslogSink::SyslogSink(const std::string& ident_, int option_, int facility_)
    : ident(ident_), option(option_), facility(facility_) {
    openlog(ident.empty() ? nullptr : ident.c_str(), option, facility);
}

SyslogSink::~SyslogSink() {
    closelog();
}

int SyslogSink::map_level(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::Trace:
        case LogLevel::Debug: return LOG_DEBUG;
        case LogLevel::Info: return LOG_INFO;
        case LogLevel::Warn: return LOG_WARNING;
        case LogLevel::Error: return LOG_ERR;
        case LogLevel::Critical: return LOG_CRIT;
        default: return LOG_INFO;
    }
}

void SyslogSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    int prio = map_level(level);
    std::string out = logger_name.empty() ? message : (logger_name + ": " + message);
    syslog(prio, "%s", out.c_str());
}

}
