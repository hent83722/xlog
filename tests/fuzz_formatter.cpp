#include <cstdint>
#include <cstddef>
#include <string>
#include <map>
#include <memory>
#include <xlog/formatter.hpp>
#include <xlog/sinks/structured_json_sink.hpp>
#include <xlog/log_level.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string s(reinterpret_cast<const char*>(data), size);


    try {
        xlog::Formatter fmt;
        fmt.format("fuzz_logger", xlog::LogLevel::Info, s);
    } catch (...) {

    }

   
    try {
       
        xlog::StructuredJsonSink sink("/tmp/fuzz_xlog.log");
        std::map<std::string, std::string> fields;
        fields["fuzz_key"] = s;
        sink.log_with_fields("fuzz_logger", xlog::LogLevel::Warn, s, fields);
    } catch (...) {
       
    }

    return 0;
}
