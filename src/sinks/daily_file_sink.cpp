#include "Zyrnix/sinks/daily_file_sink.hpp"
#include "Zyrnix/sinks/file_sink.hpp"
#include <filesystem>
namespace fs = std::filesystem;

namespace Zyrnix {

DailyFileSink::DailyFileSink(const std::string& base) : base_name(base) {
    current_date = get_date();
    open_file();
}

std::string DailyFileSink::get_date() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_r(&t, &tm_buf);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_buf);
    return std::string(buf);
}

void DailyFileSink::open_file() {
    file.open(base_name + "_" + current_date + ".log", std::ios::app);
}

void DailyFileSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    if (level < get_level()) return;
    std::lock_guard<std::mutex> lock(mtx);
    std::string today = get_date();
    if (today != current_date) {
        current_date = today;
        file.close();
        open_file();
    }
    file << formatter.format(logger_name, level, message) << std::endl;
}

}
