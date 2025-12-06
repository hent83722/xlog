#pragma once
#include "file_sink.hpp"
#include <string>
#include <chrono>
#include <ctime>

namespace xlog {

class DailyFileSink : public LogSink {
public:
    explicit DailyFileSink(const std::string& base_name);
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;

private:
    std::string base_name;
    std::ofstream file;
    std::mutex mtx;
    std::string current_date;
    void open_file();
    std::string get_date();
};

}
