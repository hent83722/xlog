# Architecture

XLog is designed to be lightweight, modular, and easy to extend. The main components are:

- **Logger**: the central object representing a named logging instance. A `Logger` owns a list of sinks and provides convenience methods for each log level (`trace`, `debug`, `info`, ...).
- **LogSink**: abstract base for output backends. Concrete sinks implement `log(name, level, message)` and may maintain internal state (files, sockets, buffers).
- **Formatter**: converts a log record (timestamp, name, level, message) into a textual representation. Formatters are used by many sinks; structured sinks may bypass the formatter to produce JSON.
- **Async subsystem**: optional thread-pool and queue used for asynchronous log dispatch to avoid blocking application threads.

Data flow
---------

1. Application calls `logger->info("...")` or uses stream-style logging.
2. `Logger` creates a log record and dispatches to each configured `LogSink`.
3. Each sink decides whether to format and write the record, or buffer it for async delivery.

Design principles
-----------------

- Minimal dependencies: standard C++17 and optional `fmt`/`asio` only for experimental sinks.
- Extensibility: new sinks can be added by inheriting from `LogSink` and registering via `logger->add_sink(...)`.
- Safety: sinks that write shared resources use mutexes; asynchronous sinks avoid blocking the caller thread.

Diagrams
--------

See `docs/diagrams/logger_lifecycle.svg` and `docs/diagrams/sink_pipeline.svg` for visual depictions of logger creation, sink delivery, and rotation/archival flows.
