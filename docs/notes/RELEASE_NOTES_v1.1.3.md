# Zyrnix v1.1.3 Release Notes

**Release Date:** January 1, 2026  
**Release Type:** Feature Release  
**Stability:** Stable (beta)

---

## 🎯 Overview

Zyrnix v1.1.3 builds directly on the v1.1.2 hardening work and the recent additions of the Loki sink, PII redaction, and hot‑reloadable configuration. This release focuses on cloud logging ergonomics, privacy controls, and better observability of your logging setup in production.

Key themes:
- Smarter, configurable Loki sink for Grafana Loki
- Stronger, more flexible PII/sensitive-data redaction
- Safer and more transparent config hot‑reloads
- Better metrics and health hooks around sinks and reloads
- An updated sample project that demonstrates the new capabilities end‑to‑end

---

## 🌐 Loki Sink Enhancements

Building on the v1.1.2 Loki sink addition, v1.1.3 makes it production‑ready.

### 1. Configurable Batching & Flush

**New capabilities:**
- `LokiOptions` struct to configure sink behaviour:
  - `batch_size` – max entries before a push (default: 10)
  - `flush_interval_ms` – max age of a batch before automatic flush (0 = disabled)
  - `timeout_ms` – CURL request timeout in milliseconds
  - `insecure_skip_verify` – disable TLS verification (for dev/test only)
  - `ca_cert_path` – custom CA bundle path
- Time‑ and size‑based triggers are combined: whichever fires first flushes the batch.

**Config JSON (per‑logger):**
```json
{
  "name": "main",
  "level": "info",
  "async": false,
  "sinks": [
    {"type": "stdout"},
    {"type": "file", "path": "main.log"},
    {
      "type": "loki",
      "url": "http://localhost:3100/loki/api/v1/push",
      "labels": "{job=\"Zyrnixtest\"}",
      "batch_size": 5,
      "flush_interval_ms": 2000,
      "timeout_ms": 5000,
      "insecure_skip_verify": false,
      "ca_cert_path": ""
    }
  ]
}
```

### 2. Retries with Exponential Backoff

When Loki is unreachable or returns a transport error:
- `LokiSink` retries up to 3 times with exponential backoff (100 ms, 200 ms, 400 ms).
- Each failed attempt is logged to `stderr` with the CURL error message.
- After the final failure, the batch is dropped to avoid unbounded memory growth.

This behaviour is contained entirely within the sink and does not require changes to existing logger code.

### 3. TLS & Timeouts

`LokiSink` now respects the following options:
- `timeout_ms` → sets `CURLOPT_TIMEOUT_MS`.
- `ca_cert_path` → sets `CURLOPT_CAINFO` for custom CAs.
- `insecure_skip_verify` → disables `CURLOPT_SSL_VERIFYPEER` and `CURLOPT_SSL_VERIFYHOST` (use only in non‑production).

These options are fully configurable via JSON and participate in hot‑reload (new sink instances created from reloaded config pick up the new settings).

### 4. Cloud Sink Identification

All cloud sinks now report themselves as such:
- `LogSink::is_cloud_sink()` virtual added (default `false`).
- `CloudWatchSink`, `AzureMonitorSink`, and `LokiSink` override it to return `true`.

This is used by the logger to apply cloud‑only redaction policies and will be useful for health/metrics routing.

---

## 🔐 Advanced PII Redaction

v1.1.3 extends the basic substring PII redaction introduced alongside the Loki sink.

### 1. Regex-Based Redaction

**New Logger APIs:**
```cpp
class Logger {
public:
    // Existing substring-based redaction
    void set_redact_patterns(const std::vector<std::string>& patterns);
    void clear_redact_patterns();

    // v1.1.3: Regex-based redaction
    void set_redact_regex_patterns(const std::vector<std::string>& patterns);

    // v1.1.3: Named PII presets (see below)
    void set_redact_pii_presets(const std::vector<std::string>& presets);

    // v1.1.3: Apply redaction only to cloud sinks or to all sinks
    void set_redact_apply_to_cloud_only(bool cloud_only);
};
```

**Behaviour:**
- Substring patterns are applied first using `Formatter::redact(...)`.
- Regex patterns are then applied via `std::regex_replace`.
- Invalid regexes are ignored to avoid throwing on the hot logging path.

### 2. Built-in PII Presets

**Supported preset names (case-insensitive):**
- `"email"` – email addresses
- `"ipv4"` – IPv4 addresses
- `"credit_card"` – simple 13–16 digit card patterns
- `"ssn"` – US social security numbers (`XXX-XX-XXXX`)

Each preset expands to a hardened regex that is applied as a redaction rule. Presets can be combined with explicit regexes and substrings.

### 3. Per-Sink Redaction (Cloud-Only Option)

Using the new `is_cloud_sink()` hook, the logger can route redaction selectively:
- When `set_redact_apply_to_cloud_only(true)` is set:
  - Cloud sinks (Loki, CloudWatch, Azure) see redacted messages.
  - Local sinks (stdout, file, rotating, etc.) see the original, unredacted message.
- When `false` (default):
  - All sinks receive the redacted message.

This makes it easy to:
- Keep full‑fidelity logs on local disks for debugging.
- Ensure only redacted content leaves your environment via cloud sinks.

### 4. Redaction via JSON Config

`LoggerConfig` gained simple redaction fields interpreted by `ConfigLoader`:
```json
{
  "name": "main",
  "level": "info",
  "async": false,
  "redact_substrings": "secret,password123",
  "redact_regexes": "[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}",
  "redact_presets": "email,ipv4",
  "redact_cloud_only": true,
  "sinks": [
    {"type": "stdout"},
    {"type": "loki", "url": "http://localhost:3100/loki/api/v1/push", "labels": "{job=\"Zyrnixtest\"}"}
  ]
}
```

- Comma‑separated lists are parsed into pattern vectors.
- All settings participate in hot‑reload via `HotReloadManager`.

---

## 🔄 Hot-Reload & Config Validation Improvements

### 1. Structured Config Errors

`ConfigLoader` now tracks a human‑readable error string when parsing fails:
```cpp
class ConfigLoader {
public:
    static bool load_from_json(const std::string& path);
    static std::string get_last_error(); // v1.1.3
};
```

Typical error messages:
- `"Could not open config file: ../log_config.json"`
- `"Missing \"loggers\" array in configuration"`
- `"Malformed configuration: expected '[' after \"loggers\""`
- `"Malformed logger object in configuration"`
- `"No valid logger configurations found"`

### 2. HotReloadManager Metrics & Logging

`HotReloadManager` now:
- Logs structured errors including the underlying `ConfigLoader` reason.
- Tracks simple reload metrics:
  - `reload_success_count()`
  - `reload_failure_count()`
  - `last_reload_time()`

Example log on failure:
```text
Failed to reload config: ../log_config.json (reason: Missing "loggers" array in configuration)
```

### 3. Dry-Run Config Lint Mode (Sample Project)

The sample project gained a lightweight lint mode so you can validate JSON without starting your app:

```bash
cd sample_project/build
./sample_main --lint-config
```

- Exit code `0` → configuration valid
- Exit code `1` → invalid; prints `ConfigLoader::get_last_error()`

### 4. Live Tuning of Loki via Hot-Reload

`sample_project/log_config.json` now defines a Loki sink on the `main` logger with `batch_size` and `flush_interval_ms`. Editing these fields while the sample runs and waiting for the watcher interval will update the behaviour without restart.

The sample prints simple statistics after reload:
```text
Reload successes: <n>, failures: <m>
```

---

## 📊 Reliability & Observability

### 1. Sink Metrics Integration for Loki

`LokiSink` now integrates with the existing `MetricsRegistry`:
- Each successful batch flush records a sink flush event.
- Each failed send attempt records a sink error.

You can export metrics via:
```cpp
auto& registry = MetricsRegistry::instance();
std::string prom = registry.export_all_prometheus("Zyrnix");
std::string json = registry.export_all_json();
```

These metrics complement the existing logger‑level `LogMetrics` and health APIs introduced in earlier releases.

### 2. Health & Cloud Awareness

By standardising on `LogSink::is_cloud_sink()` for cloud sinks:
- Health/metrics UI and tooling can treat cloud sinks specially (e.g., stricter error budgets).
- Redaction routing (cloud‑only vs all‑sinks) becomes straightforward and reliable.

---

## 🧪 Sample Project Updates

The `sample_project` was updated to showcase the new features.

### 1. Updated Config

`sample_project/log_config.json`:
- `main` logger now has:
  - `stdout` sink
  - `file` sink (`main.log`)
  - `loki` sink configured with URL, labels, batch size, and flush interval
- Demonstrates how to configure Loki entirely via JSON.

### 2. New Behaviours in main.cpp

The sample now demonstrates:
- **PII redaction modes:**
  - Substring redaction
  - Regex‑based redaction
  - Built‑in presets (`email`, `ipv4`) applied cloud‑only
- **Loki connectivity:**
  - Messages sent through the config‑defined Loki sink
  - Visible retry/backoff behaviour when the endpoint is unreachable
- **Hot‑reload:**
  - Live tuning of `batch_size` / `flush_interval_ms` with log messages before/after reload
  - Simple reload success/failure counters printed to stdout
- **Config linting:**
  - `--lint-config` mode for pre‑deployment validation

---

## 📦 Summary of New APIs (v1.1.3)

### Logger
```cpp
class Logger {
public:
    // Redaction
    void set_redact_patterns(const std::vector<std::string>& patterns);
    void clear_redact_patterns();
    void set_redact_regex_patterns(const std::vector<std::string>& patterns);   // v1.1.3
    void set_redact_pii_presets(const std::vector<std::string>& presets);       // v1.1.3
    void set_redact_apply_to_cloud_only(bool cloud_only);                        // v1.1.3
};
```

### LokiSink
```cpp
struct LokiOptions {
    size_t  batch_size        = 10;
    uint64_t flush_interval_ms = 0;
    long    timeout_ms        = 5000;
    bool    insecure_skip_verify = false;
    std::string ca_cert_path;
};

class LokiSink : public LogSink {
public:
    LokiSink(const std::string& url,
             const std::string& labels = "",
             const LokiOptions& opts = LokiOptions());

    void log(const std::string& name, LogLevel level, const std::string& message) override;
    void flush();

    bool is_cloud_sink() const override; // returns true

    void set_options(const LokiOptions& opts); // runtime updates
};
```

### ConfigLoader
```cpp
struct LoggerConfig {
    std::string name;
    LogLevel level = LogLevel::Info;
    bool async = false;
    std::vector<std::string> sinks;
    std::map<std::string, std::string> sink_params;

    // v1.1.3 redaction helpers
    std::string redact_substrings;
    std::string redact_regexes;
    std::string redact_presets;
    bool redact_cloud_only = false;
};

class ConfigLoader {
public:
    static bool load_from_json(const std::string& path);
    static bool load_from_json_string(const std::string& json_str);
    static std::vector<LoggerConfig> get_logger_configs();
    static std::map<std::string, std::shared_ptr<Logger>> create_loggers();
    static void clear();

    static std::string get_last_error(); // v1.1.3
};
```

### HotReloadManager
```cpp
class HotReloadManager {
public:
    HotReloadManager(const std::string& config_path);
    ~HotReloadManager();

    void start();
    void stop();

    std::shared_ptr<Logger> get_logger(const std::string& name);
    std::map<std::string, std::shared_ptr<Logger>> get_all_loggers();

    // v1.1.3 metrics
    uint64_t reload_success_count() const;
    uint64_t reload_failure_count() const;
    std::chrono::system_clock::time_point last_reload_time() const;
};
```

---

## 🔁 Relationship to Previous Work

This release builds on the earlier commit that introduced:
- A basic `LokiSink` cloud sink targeting Grafana Loki via libcurl.
- Substring‑based PII/sensitive data redaction through `Formatter::redact(...)` and `Logger::set_redact_patterns(...)`.
- A polling‑based `ConfigWatcher` plus `HotReloadManager` for JSON‑driven logger creation and hot‑reload.
- A sample project demonstrating PII masking, Loki integration, and hot‑reload.

v1.1.3 layers configurability, safety, and observability on top of those foundations without breaking existing semantics or configuration formats.

---

**Issues & Feedback:** Please file reports and feature requests on the GitHub issue tracker.
