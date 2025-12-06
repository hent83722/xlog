#pragma once
#include "file_sink.hpp"
#include <string>
#include <cstddef>

namespace xlog {

class RotatingFileSink : public LogSink {
public:
    RotatingFileSink(const std::string& base_name, size_t max_size, size_t max_files);
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;

private:
    std::string base_name;
    size_t max_size;
    size_t max_files;
    size_t current_size = 0;
    std::ofstream file;
    std::mutex mtx;
    void rotate();
    void open_file();
};

}
