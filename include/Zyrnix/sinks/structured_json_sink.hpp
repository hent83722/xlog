#pragma once
#include "../log_sink.hpp"
#include "../log_level.hpp"
#include <string>
#include <map>
#include <fstream>
#include <mutex>
#include <memory>

namespace Zyrnix {


class StructuredJsonSink : public LogSink {
public:
    explicit StructuredJsonSink(const std::string& filename);
    ~StructuredJsonSink();
    
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;
    

    void set_context(const std::string& key, const std::string& value);

    void log_with_fields(const std::string& logger_name, LogLevel level, 
                         const std::string& message,
                         const std::map<std::string, std::string>& fields);
    

    void clear_context();

private:
    std::string filename;
    std::map<std::string, std::string> global_context;
    std::ofstream file;
    std::mutex mtx;
    
    std::string build_json(const std::string& logger_name, LogLevel level,
                          const std::string& message,
                          const std::map<std::string, std::string>& fields);
};

}
