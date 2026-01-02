# Zyrnix Documentation

Welcome to the Zyrnix documentation. This collection of pages explains architecture, configuration, sinks, threading and performance considerations, examples, and troubleshooting tips for integrating Zyrnix into your projects.

Quick links:

- **Architecture:** `architecture.md` — high-level design and components
- **Configuration:** `config.md` — runtime and build-time configuration
- **Sinks:** `sinks.md` — available output backends and how to extend them
- **Threading:** `threading.md` — thread-safety and asynchronous logging
- **Performance:** `performance.md` — benchmarks and tuning guidance
- **Examples:** `examples.md` — sample code and patterns
- **Troubleshooting:** `troubleshooting.md` — common problems and fixes
- **FAQ:** `faq.md` — frequently asked questions

Getting started
--------------

1. Build the library:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

2. Add Zyrnix to your project:

```cmake
add_subdirectory(path/to/Zyrnix)
target_link_libraries(your_project PRIVATE Zyrnix)
target_include_directories(your_project PRIVATE path/to/Zyrnix/include)
```

3. See `examples.md` for concrete usage patterns and recommended sink setups.

If you need a concise release summary, see `docs/releases/v1.0.1.md`.
