# Quick Manual Testing Guide for Zyrnix v1.0.3

## Quick Test (5 minutes)

### 1. Build the project
```bash
cd /home/henri/Zyrnix
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build . --parallel
```
**Expected:** Clean build with no errors

### 2. Run unit tests
```bash
./tests/tests
```
**Expected:** All tests pass

### 3. Test each example quickly
```bash
# Basic logging
g++ -std=c++17 -I../include -L. -o test1 ../examples/basic_logging.cpp -lZyrnix -lpthread && ./test1

# Async logging  
g++ -std=c++17 -I../include -L. -o test2 ../examples/async_logging.cpp -lZyrnix -lpthread && ./test2

# Context logging (NEW FEATURE)
g++ -std=c++17 -I../include -L. -o test3 ../examples/context_logging.cpp -lZyrnix -lpthread && ./test3

# Structured JSON
g++ -std=c++17 -I../include -L. -o test4 ../examples/structured_json_example.cpp -lZyrnix -lpthread && ./test4
```
**Expected:** All compile and run without errors

### 4. Verify context feature output
```bash
cat payments.jsonl | head -2
```
**Expected:** JSON with `payment_id`, `service`, `amount`, `currency` fields

### 5. Run sanitizer test
```bash
cd .. && bash local_test/run_asan.sh
```
**Expected:** No memory leaks or errors

---

## Comprehensive Test (Automated)

Run the full test suite:
```bash
cd /home/henri/Zyrnix
bash test_all_features.sh
```

This tests:
- ✓ Clean build
- ✓ Unit tests
- ✓ All 10 examples compile and run
- ✓ New context feature
- ✓ All sinks (stdout, file, rotating, daily, null, JSON)
- ✓ Async logging
- ✓ Log levels and filtering
- ✓ Thread safety (10 threads, 1000 messages)
- ✓ Memory leak check (valgrind)
- ✓ AddressSanitizer

---

## Feature Checklist

### Core Features
- [ ] Basic logging (trace, debug, info, warn, error, critical)
- [ ] Stream-style logging (`*logger << Info << "msg" << endl`)
- [ ] Multiple sinks
- [ ] Async logging
- [ ] Thread safety
- [ ] Log level filtering

### Sinks
- [ ] StdoutSink
- [ ] FileSink
- [ ] RotatingFileSink
- [ ] DailyFileSink
- [ ] NullSink
- [ ] StructuredJsonSink
- [ ] MultiSink
- [ ] SyslogSink (Linux only)
- [ ] UdpSink

### New in v1.0.3
- [ ] LogContext (global context)
- [ ] ScopedContext (automatic RAII)
- [ ] Thread-local context storage
- [ ] Context field injection
- [ ] Nested contexts
- [ ] Chainable context API

### Quality Assurance
- [ ] AddressSanitizer tests
- [ ] ThreadSanitizer tests
- [ ] UndefinedBehaviorSanitizer tests
- [ ] Fuzz testing
- [ ] CI integration

---

## Quick Smoke Test (1 minute)

```bash
cd /home/henri/Zyrnix/build

# Test 1: Build
cmake --build . --parallel && echo "✓ Build passed" || echo "✗ Build failed"

# Test 2: Unit tests
./tests/tests && echo "✓ Tests passed" || echo "✗ Tests failed"

# Test 3: Context feature
g++ -std=c++17 -I../include -L. ../examples/context_logging.cpp -lZyrnix -lpthread -o ctx && \
./ctx && \
[ -f payments.jsonl ] && \
grep -q "payment_id" payments.jsonl && \
echo "✓ Context feature works" || echo "✗ Context feature failed"
```

---

## If Something Breaks

### Build fails
```bash
cd /home/henri/Zyrnix
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel --verbose
```

### Example fails to compile
```bash
# Check library was built
ls -lh build/libZyrnix.a

# Try manual compile with verbose output
g++ -std=c++17 -I include -L build -o test examples/basic_logging.cpp -lZyrnix -lpthread -v
```

### Context feature not working
```bash
# Check header exists
ls -lh include/Zyrnix/log_context.hpp

# Check implementation compiled
nm build/libZyrnix.a | grep LogContext

# Run with debug output
./context_example
cat payments.jsonl
```

### Memory leaks
```bash
valgrind --leak-check=full ./context_example
```

---

## Expected Test Results

All tests should show:
```
✓ CMake configuration
✓ Project compilation  
✓ Unit tests execution
✓ Compiled basic_logging
✓ Executed basic_logging
✓ Compiled async_logging
✓ Executed async_logging
... (all examples)
✓ Context tests passed
✓ All sinks working
✓ Thread safety verified
✓ No memory leaks

========================================
ALL TESTS PASSED! ✓
Zyrnix v1.0.3 is working correctly!
========================================
```

If you see **"ALL TESTS PASSED!"** — everything works! 🎉
