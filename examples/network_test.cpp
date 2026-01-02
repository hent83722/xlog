#include "Zyrnix/logger.hpp"
#include "Zyrnix/sinks/stdout_sink.hpp"
#include "Zyrnix/sinks/file_sink.hpp"
#include "Zyrnix/sinks/multi_sink.hpp"
#include "Zyrnix/experimental/network_sink.hpp"

int main() {
    using namespace Zyrnix;

    auto stdout_sink = std::make_shared<StdoutSink>();
    auto file_sink = std::make_shared<FileSink>("logs.txt");
    auto network_sink = std::make_shared<NetworkSink>("192.168.1.100:9000"); // your phone's IP

    MultiSink multi_sink;
    multi_sink.add_sink(stdout_sink);
    multi_sink.add_sink(file_sink);
    multi_sink.add_sink(network_sink);

    Logger logger("network_logger", std::make_shared<MultiSink>(multi_sink));

    logger.info("Hello, this log goes to PC terminal, file, AND phone!");
    logger.warn("Warning: check your phone!");

    return 0;
}
