#pragma once
#include "log_sink.hpp"
#include <fstream>
#include <string>
#include <mutex>

namespace xlog {

class FileSink : public LogSink {
public:
    explicit FileSink(const std::string& filename);
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;

private:
    std::ofstream file;
    std::mutex mtx;
};

}
