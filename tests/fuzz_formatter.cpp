#include <cstdint>
#include <cstddef>
#include <string>
#include <map>
#include <memory>
#include <Zyrnix/formatter.hpp>
#include <Zyrnix/sinks/structured_json_sink.hpp>
#include <Zyrnix/log_level.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string s(reinterpret_cast<const char*>(data), size);


    try {
        Zyrnix::Formatter fmt;
        fmt.format("fuzz_logger", Zyrnix::LogLevel::Info, s);
    } catch (...) {

    }

   
    try {
       
        Zyrnix::StructuredJsonSink sink("/tmp/fuzz_Zyrnix.log");
        std::map<std::string, std::string> fields;
        fields["fuzz_key"] = s;
        sink.log_with_fields("fuzz_logger", Zyrnix::LogLevel::Warn, s, fields);
    } catch (...) {
       
    }

    return 0;
}
