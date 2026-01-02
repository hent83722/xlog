#include <memory>
#include <Zyrnix/logger.hpp>
#include <Zyrnix/sinks/udp_sink.hpp>
#include <Zyrnix/sinks/syslog_sink.hpp>

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <udp-host> <udp-port>\n", argv[0]);
        return 1;
    }

    std::string host = argv[1];
    unsigned short port = static_cast<unsigned short>(std::stoi(argv[2]));

    auto logger = std::make_shared<Zyrnix::Logger>("network_syslog_logger");

    // UDP sink (send messages to a UDP log collector)
    auto udp = std::make_shared<Zyrnix::UdpSink>(host, port);
    logger->add_sink(udp);

    // Syslog sink (send to system syslog)
    auto sys = std::make_shared<Zyrnix::SyslogSink>("Zyrnix_example", LOG_PID, LOG_USER);
    logger->add_sink(sys);

    logger->info("This is an info message to UDP and syslog");
    logger->error("This is an error message");

    return 0;
}
