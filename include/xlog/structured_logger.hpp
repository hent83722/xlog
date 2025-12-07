#pragma once
#include "logger.hpp"
#include "sinks/structured_json_sink.hpp"
#include <map>
#include <memory>

namespace xlog {


class StructuredLogger {
public:
  
    static std::shared_ptr<StructuredLogger> create(const std::string& name, const std::string& filename);
    
    StructuredLogger(std::shared_ptr<Logger> logger, std::shared_ptr<StructuredJsonSink> sink);
    
  
    void set_context(const std::string& key, const std::string& value);
    
  
    void clear_context();
    
   
    void trace(const std::string& message, const std::map<std::string, std::string>& fields = {});
    void debug(const std::string& message, const std::map<std::string, std::string>& fields = {});
    void info(const std::string& message, const std::map<std::string, std::string>& fields = {});
    void warn(const std::string& message, const std::map<std::string, std::string>& fields = {});
    void error(const std::string& message, const std::map<std::string, std::string>& fields = {});
    void critical(const std::string& message, const std::map<std::string, std::string>& fields = {});

private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<StructuredJsonSink> json_sink;
};

}
