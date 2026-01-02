#include "Zyrnix/sinks/rotating_file_sink.hpp"
#include "Zyrnix/sinks/file_sink.hpp"
#include "Zyrnix/util.hpp"
#include <filesystem>
namespace fs = std::filesystem;

namespace Zyrnix {

RotatingFileSink::RotatingFileSink(const std::string& base, size_t max_s, size_t max_f)
    : base_name(base), max_size(max_s), max_files(max_f) {
    open_file();
}

void RotatingFileSink::open_file() {
    std::string filename = base_name + ".log";
    
   
#ifdef _WIN32
    std::wstring wpath = path::to_native(filename);
    file.open(wpath, std::ios::app);
    if (file.is_open()) {
        current_size = std::filesystem::file_size(fs::path(wpath));
    }
#else
    file.open(filename, std::ios::app);
    if (file.is_open()) {
        current_size = std::filesystem::file_size(filename);
    }
#endif
}

void RotatingFileSink::rotate() {
    file.close();
    

    for (size_t i = max_files; i > 0; --i) {
        std::string old_name = base_name + "." + std::to_string(i-1) + ".log";
        std::string new_name = base_name + "." + std::to_string(i) + ".log";
        
#ifdef _WIN32
        fs::path old_path(path::to_native(old_name));
        fs::path new_path(path::to_native(new_name));
        if (fs::exists(old_path)) {
            std::error_code ec;
            fs::rename(old_path, new_path, ec);
        }
#else
        if (fs::exists(old_name)) {
            std::error_code ec;
            fs::rename(old_name, new_name, ec);
        }
#endif
    }
    
    std::string current_file = base_name + ".log";
    std::string rotated_file = base_name + ".0.log";
    
#ifdef _WIN32
    fs::path current_path(path::to_native(current_file));
    fs::path rotated_path(path::to_native(rotated_file));
    std::error_code ec;
    fs::rename(current_path, rotated_path, ec);
#else
    std::error_code ec;
    fs::rename(current_file, rotated_file, ec);
#endif
    
    open_file();
}

void RotatingFileSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    if (level < get_level()) return;
    std::lock_guard<std::mutex> lock(mtx);
    std::string line = formatter.format(logger_name, level, message) + "\n";
    file << line;
    current_size += line.size();
    if (current_size >= max_size) rotate();
}

}
