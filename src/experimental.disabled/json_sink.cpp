#include "Zyrnix/experimental/json_sink.hpp"
#include "Zyrnix/sinks/file_sink.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <mutex>

namespace Zyrnix {

JsonSink::JsonSink(const std::string& filename) {
    file.open(filename, std::ios::app);
}

void JsonSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    if (level < get_level()) return;
    std::lock_guard<std::mutex> lock(mtx);
    nlohmann::json j;
    j["logger"] = logger_name;
    j["level"] = to_string(level);
    j["message"] = message;
    file << j.dump() << std::endl;
}

}
