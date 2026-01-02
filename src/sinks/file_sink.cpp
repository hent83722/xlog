#include "Zyrnix/sinks/file_sink.hpp"
#include "Zyrnix/log_sink.hpp"
#include "Zyrnix/util.hpp"
#include <fstream>
#include <mutex>

namespace Zyrnix {

FileSink::FileSink(const std::string& filename) {
#ifdef _WIN32
    std::wstring wpath = path::to_native(filename);
    file.open(wpath, std::ios::app);
#else
    file.open(filename, std::ios::app);
#endif
}

void FileSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    if (level < get_level()) return;
    std::lock_guard<std::mutex> lock(mtx);
    if (file.is_open()) {
        file << formatter.format(logger_name, level, message) << std::endl;
    }
}

}
