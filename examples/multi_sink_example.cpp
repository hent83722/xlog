#include "Zyrnix/logger.hpp"
#include "Zyrnix/sinks/stdout_sink.hpp"
#include "Zyrnix/sinks/file_sink.hpp"
#include "Zyrnix/sinks/multi_sink.hpp"
#include "Zyrnix/experimental/network_sink.hpp"

int main() {
    using namespace Zyrnix;

    // Create sinks
    auto stdout_sink = std::make_shared<StdoutSink>();
    auto file_sink = std::make_shared<FileSink>("logs.txt");
    auto network_sink = std::make_shared<NetworkSink>("127.0.0.1:9000");

    // Combine them into a MultiSink
    MultiSink multi_sink;
    multi_sink.add_sink(stdout_sink);
    multi_sink.add_sink(file_sink);
    multi_sink.add_sink(network_sink);

    // Use MultiSink with a logger
    Logger logger("multi_logger", std::make_shared<MultiSink>(multi_sink));

    logger.info("This will go to stdout, file, and network!");
    logger.warn("Warning message sent everywhere!");

    return 0;
}
