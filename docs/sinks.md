# Sinks / Output Backends

XLog supports multiple sinks out of the box. In addition to console and file sinks, this release adds two additional, commonly-requested backends:

- **SyslogSink** — sends log messages to the system syslog using the POSIX `syslog` API. Useful for integration with system logging on Linux/Unix hosts.
- **UdpSink** — lightweight UDP network sink that sends log messages to a remote collector (e.g. log aggregator, forwarder).

Public headers (new):

- `include/xlog/sinks/syslog_sink.hpp`
- `include/xlog/sinks/udp_sink.hpp`

Example usage (see `examples/network_syslog_example.cpp`):

```
auto logger = std::make_shared<xlog::Logger>("my_logger");
auto udp = std::make_shared<xlog::UdpSink>("127.0.0.1", 5140);
auto sys = std::make_shared<xlog::SyslogSink>("myapp", LOG_PID, LOG_USER);
logger->add_sink(udp);
logger->add_sink(sys);
logger->info("Startup complete");
```

Notes and next steps:

- The `UdpSink` is intentionally simple (stateless UDP `sendto`) for low overhead. For TCP or reliable delivery, consider adding a TCP sink or using the existing experimental `network_sink` (which uses ASIO).
- The `SyslogSink` uses the system `openlog`/`syslog` API. On platforms without POSIX syslog, this sink will not be available.
- Consider adding an optional CMake flag to enable/disable experimental or platform-specific sinks.
