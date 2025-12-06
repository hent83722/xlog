#include "xlog/sinks/file_sink.hpp"
#include "xlog/log_sink.hpp"
#include <fstream>
#include <mutex>

namespace xlog {

FileSink::FileSink(const std::string& filename) {
    file.open(filename, std::ios::app);
}

void FileSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    if (level < get_level()) return;
    std::lock_guard<std::mutex> lock(mtx);
    if (file.is_open()) {
        file << formatter.format(logger_name, level, message) << std::endl;
    }
}

}
