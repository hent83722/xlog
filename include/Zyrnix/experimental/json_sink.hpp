#pragma once
#include "log_sink.hpp"
#include <string>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>

namespace Zyrnix {

class JsonSink : public LogSink {
public:
    explicit JsonSink(const std::string& filename);
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;

private:
    std::ofstream file;
    std::mutex mtx;
};

}
