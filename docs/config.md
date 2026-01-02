# Configuration

Zyrnix supports both compile-time and runtime configuration. This page outlines the most important options and recommended defaults.

Build-time options (CMake)
--------------------------

- `ENABLE_SYSLOG` (ON/OFF) — controls whether `SyslogSink` is built into the library. Default: ON for Unixlike systems, OFF on Windows.
- `CMAKE_BUILD_TYPE` — standard CMake `Release`/`Debug` selection.

Runtime configuration
---------------------

The library exposes no global configuration file by default; instead, create and attach sinks programmatically at application startup. A simple pattern:

```cpp
auto logger = std::make_shared<Zyrnix::Logger>("service");
if (config.console) logger->add_sink(std::make_shared<Zyrnix::StdoutSink>());
if (config.file_path) logger->add_sink(std::make_shared<Zyrnix::FileSink>(config.file_path));
if (config.udp.host) logger->add_sink(std::make_shared<Zyrnix::UdpSink>(config.udp.host, config.udp.port));
```

Recommended configuration keys
------------------------------

- `log_level` — default minimum level (Info) to reduce noise in production
- `sinks` — list of sinks to enable (stdout, file, rotating_file, syslog, udp)
- `structured` — enable structured JSON logging for aggregator ingestion

Environment variables
---------------------

You can optionally control log level and sink selection via environment variables if you prefer zero-configuration deployments:

```bash
export XLOG_LEVEL=info
export XLOG_ENABLE_SYSLOG=1
```

Implementations should read these variables during startup and configure loggers accordingly.
