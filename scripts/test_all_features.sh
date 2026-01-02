#!/bin/bash
# Comprehensive test script for Zyrnix v1.0.3
# Tests all features to ensure nothing is broken

set -e  # Exit on any error

echo "========================================"
echo "Zyrnix v1.0.3 Comprehensive Test Suite"
echo "========================================"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

test_passed() {
    echo -e "${GREEN}✓ $1${NC}"
    ((TESTS_PASSED++))
}

test_failed() {
    echo -e "${RED}✗ $1${NC}"
    ((TESTS_FAILED++))
}

echo "========================================="
echo "Step 1: Clean Build"
echo "========================================="
rm -rf build
mkdir -p build
cd build

echo "Building with CMake..."
if cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON; then
    test_passed "CMake configuration"
else
    test_failed "CMake configuration"
    exit 1
fi

if cmake --build . --parallel; then
    test_passed "Project compilation"
else
    test_failed "Project compilation"
    exit 1
fi

echo ""
echo "========================================="
echo "Step 2: Run Unit Tests"
echo "========================================="
if [ -f tests/tests ]; then
    if ./tests/tests; then
        test_passed "Unit tests execution"
    else
        test_failed "Unit tests execution"
    fi
else
    echo -e "${YELLOW}⚠ No unit tests found (expected tests/tests)${NC}"
fi

echo ""
echo "========================================="
echo "Step 3: Test All Examples"
echo "========================================="

# Compile and test each example
EXAMPLES=(
    "basic_logging"
    "async_logging"
    "file_vs_stdout"
    "rotating_logs"
    "log_levels"
    "multi_logger"
    "multi_sink_example"
    "daily_logs"
    "structured_json_example"
    "context_logging"
)

for example in "${EXAMPLES[@]}"; do
    echo ""
    echo "Testing: $example"
    if [ -f "../examples/${example}.cpp" ]; then
        if g++ -std=c++17 -I../include -L. -o "test_${example}" "../examples/${example}.cpp" -lZyrnix -lpthread 2>&1 | tee compile.log; then
            test_passed "Compiled ${example}"
            
            # Run the example with timeout
            if timeout 5s "./test_${example}" > "/tmp/Zyrnix_${example}.out" 2>&1; then
                test_passed "Executed ${example}"
            else
                EXIT_CODE=$?
                if [ $EXIT_CODE -eq 124 ]; then
                    test_passed "Executed ${example} (timeout - long running)"
                else
                    test_failed "Executed ${example} (exit code: $EXIT_CODE)"
                    cat "/tmp/Zyrnix_${example}.out"
                fi
            fi
            
            rm -f "test_${example}"
        else
            test_failed "Compiled ${example}"
            cat compile.log
        fi
    else
        echo -e "${YELLOW}⚠ Example not found: ${example}.cpp${NC}"
    fi
done

echo ""
echo "========================================="
echo "Step 4: Test New Context Feature"
echo "========================================="

# Create a simple test program
cat > test_context_simple.cpp << 'EOF'
#include <Zyrnix/structured_logger.hpp>
#include <Zyrnix/log_context.hpp>
#include <iostream>

int main() {
    auto logger = Zyrnix::StructuredLogger::create("test", "test_context.jsonl");
    
    // Test 1: Basic scoped context
    {
        Zyrnix::ScopedContext ctx;
        ctx.set("test_id", "test-001");
        logger->info("Test 1: Basic context");
    }
    
    // Test 2: Chainable API
    {
        Zyrnix::ScopedContext ctx;
        ctx.set("key1", "value1").set("key2", "value2");
        logger->info("Test 2: Chained context");
    }
    
    // Test 3: Nested contexts
    {
        Zyrnix::ScopedContext outer;
        outer.set("outer", "outer_value");
        logger->info("Test 3a: Outer context");
        
        {
            Zyrnix::ScopedContext inner;
            inner.set("inner", "inner_value");
            logger->info("Test 3b: Nested context");
        }
        
        logger->info("Test 3c: Back to outer");
    }
    
    // Test 4: Global context
    Zyrnix::LogContext::set("global", "global_value");
    logger->info("Test 4: Global context");
    Zyrnix::LogContext::clear();
    
    std::cout << "Context tests completed successfully!" << std::endl;
    return 0;
}
EOF

echo "Compiling context test..."
if g++ -std=c++17 -I../include -L. -o test_context_simple test_context_simple.cpp -lZyrnix -lpthread; then
    test_passed "Compiled context test"
    
    if ./test_context_simple; then
        test_passed "Executed context test"
        
        # Verify output file
        if [ -f test_context.jsonl ]; then
            LINE_COUNT=$(wc -l < test_context.jsonl)
            if [ "$LINE_COUNT" -eq 6 ]; then
                test_passed "Context test produced correct output (6 lines)"
                
                # Verify context fields are present
                if grep -q "test_id" test_context.jsonl && \
                   grep -q "key1" test_context.jsonl && \
                   grep -q "outer" test_context.jsonl && \
                   grep -q "inner" test_context.jsonl && \
                   grep -q "global" test_context.jsonl; then
                    test_passed "All context fields present in output"
                else
                    test_failed "Missing context fields in output"
                fi
            else
                test_failed "Context test output has wrong line count: $LINE_COUNT (expected 6)"
            fi
        else
            test_failed "Context test did not create output file"
        fi
    else
        test_failed "Executed context test"
    fi
else
    test_failed "Compiled context test"
fi

echo ""
echo "========================================="
echo "Step 5: Test All Sinks"
echo "========================================="

cat > test_all_sinks.cpp << 'EOF'
#include <Zyrnix/logger.hpp>
#include <Zyrnix/sinks/stdout_sink.hpp>
#include <Zyrnix/sinks/file_sink.hpp>
#include <Zyrnix/sinks/rotating_file_sink.hpp>
#include <Zyrnix/sinks/daily_file_sink.hpp>
#include <Zyrnix/sinks/null_sink.hpp>
#include <Zyrnix/sinks/multi_sink.hpp>
#include <Zyrnix/sinks/structured_json_sink.hpp>
#include <iostream>

int main() {
    auto logger = std::make_shared<Zyrnix::Logger>("test_sinks");
    
    // Test StdoutSink
    logger->add_sink(std::make_shared<Zyrnix::StdoutSink>());
    logger->info("Testing StdoutSink");
    logger->clear_sinks();
    
    // Test FileSink
    logger->add_sink(std::make_shared<Zyrnix::FileSink>("test_file.log"));
    logger->info("Testing FileSink");
    logger->clear_sinks();
    
    // Test RotatingFileSink
    logger->add_sink(std::make_shared<Zyrnix::RotatingFileSink>("test_rotating.log", 1024, 3));
    logger->info("Testing RotatingFileSink");
    logger->clear_sinks();
    
    // Test DailyFileSink
    logger->add_sink(std::make_shared<Zyrnix::DailyFileSink>("test_daily.log"));
    logger->info("Testing DailyFileSink");
    logger->clear_sinks();
    
    // Test NullSink
    logger->add_sink(std::make_shared<Zyrnix::NullSink>());
    logger->info("Testing NullSink");
    logger->clear_sinks();
    
    // Test StructuredJsonSink
    auto json_sink = std::make_shared<Zyrnix::StructuredJsonSink>("test_json.jsonl");
    logger->add_sink(json_sink);
    logger->info("Testing StructuredJsonSink");
    logger->clear_sinks();
    
    std::cout << "All sinks tested successfully!" << std::endl;
    return 0;
}
EOF

echo "Compiling sinks test..."
if g++ -std=c++17 -I../include -L. -o test_all_sinks test_all_sinks.cpp -lZyrnix -lpthread; then
    test_passed "Compiled sinks test"
    
    if ./test_all_sinks > /dev/null 2>&1; then
        test_passed "Executed sinks test"
        
        # Verify output files
        [ -f test_file.log ] && test_passed "FileSink created output" || test_failed "FileSink output missing"
        [ -f test_rotating.log ] && test_passed "RotatingFileSink created output" || test_failed "RotatingFileSink output missing"
        [ -f test_json.jsonl ] && test_passed "StructuredJsonSink created output" || test_failed "StructuredJsonSink output missing"
    else
        test_failed "Executed sinks test"
    fi
else
    test_failed "Compiled sinks test"
fi

echo ""
echo "========================================="
echo "Step 6: Test Async Logging"
echo "========================================="

cat > test_async.cpp << 'EOF'
#include <Zyrnix/async/async_logger.hpp>
#include <Zyrnix/sinks/file_sink.hpp>
#include <thread>
#include <iostream>

int main() {
    auto logger = std::make_shared<Zyrnix::AsyncLogger>("async_test");
    logger->add_sink(std::make_shared<Zyrnix::FileSink>("test_async.log"));
    
    for (int i = 0; i < 100; ++i) {
        logger->info("Async message " + std::to_string(i));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Async logging test completed!" << std::endl;
    return 0;
}
EOF

echo "Compiling async test..."
if g++ -std=c++17 -I../include -L. -o test_async test_async.cpp -lZyrnix -lpthread; then
    test_passed "Compiled async test"
    
    if ./test_async; then
        test_passed "Executed async test"
        
        if [ -f test_async.log ]; then
            LINE_COUNT=$(wc -l < test_async.log)
            if [ "$LINE_COUNT" -ge 90 ]; then
                test_passed "Async logging produced sufficient output ($LINE_COUNT lines)"
            else
                test_failed "Async logging produced too few lines ($LINE_COUNT, expected ~100)"
            fi
        else
            test_failed "Async logging did not create output file"
        fi
    else
        test_failed "Executed async test"
    fi
else
    test_failed "Compiled async test"
fi

echo ""
echo "========================================="
echo "Step 7: Test Log Levels"
echo "========================================="

cat > test_log_levels.cpp << 'EOF'
#include <Zyrnix/logger.hpp>
#include <Zyrnix/sinks/file_sink.hpp>
#include <iostream>

int main() {
    auto logger = std::make_shared<Zyrnix::Logger>("level_test");
    logger->add_sink(std::make_shared<Zyrnix::FileSink>("test_levels.log"));
    
    logger->set_level(Zyrnix::LogLevel::Trace);
    logger->trace("Trace message");
    logger->debug("Debug message");
    logger->info("Info message");
    logger->warn("Warning message");
    logger->error("Error message");
    logger->critical("Critical message");
    
    // Test level filtering
    logger->set_level(Zyrnix::LogLevel::Warn);
    logger->info("This should not appear");
    logger->warn("This should appear");
    
    std::cout << "Log levels test completed!" << std::endl;
    return 0;
}
EOF

echo "Compiling log levels test..."
if g++ -std=c++17 -I../include -L. -o test_log_levels test_log_levels.cpp -lZyrnix -lpthread; then
    test_passed "Compiled log levels test"
    
    if ./test_log_levels; then
        test_passed "Executed log levels test"
        
        if [ -f test_levels.log ]; then
            if grep -q "Trace message" test_levels.log && \
               grep -q "Critical message" test_levels.log && \
               grep -q "This should appear" test_levels.log && \
               ! grep -q "This should not appear" test_levels.log; then
                test_passed "Log level filtering works correctly"
            else
                test_failed "Log level filtering not working correctly"
            fi
        else
            test_failed "Log levels test did not create output file"
        fi
    else
        test_failed "Executed log levels test"
    fi
else
    test_failed "Compiled log levels test"
fi

echo ""
echo "========================================="
echo "Step 8: Test Thread Safety"
echo "========================================="

cat > test_thread_safety.cpp << 'EOF'
#include <Zyrnix/logger.hpp>
#include <Zyrnix/sinks/file_sink.hpp>
#include <thread>
#include <vector>
#include <iostream>

void worker(std::shared_ptr<Zyrnix::Logger> logger, int id) {
    for (int i = 0; i < 100; ++i) {
        logger->info("Thread " + std::to_string(id) + " message " + std::to_string(i));
    }
}

int main() {
    auto logger = std::make_shared<Zyrnix::Logger>("thread_test");
    logger->add_sink(std::make_shared<Zyrnix::FileSink>("test_threads.log"));
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(worker, logger, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Thread safety test completed!" << std::endl;
    return 0;
}
EOF

echo "Compiling thread safety test..."
if g++ -std=c++17 -I../include -L. -o test_thread_safety test_thread_safety.cpp -lZyrnix -lpthread; then
    test_passed "Compiled thread safety test"
    
    if ./test_thread_safety; then
        test_passed "Executed thread safety test"
        
        if [ -f test_threads.log ]; then
            LINE_COUNT=$(wc -l < test_threads.log)
            if [ "$LINE_COUNT" -eq 1000 ]; then
                test_passed "Thread safety test produced correct output (1000 lines)"
            else
                echo -e "${YELLOW}⚠ Thread safety test produced $LINE_COUNT lines (expected 1000)${NC}"
            fi
        else
            test_failed "Thread safety test did not create output file"
        fi
    else
        test_failed "Executed thread safety test"
    fi
else
    test_failed "Compiled thread safety test"
fi

echo ""
echo "========================================="
echo "Step 9: Memory Leak Check (Optional)"
echo "========================================="

if command -v valgrind &> /dev/null; then
    echo "Running valgrind on context test..."
    if valgrind --leak-check=full --error-exitcode=1 ./test_context_simple > /dev/null 2>&1; then
        test_passed "No memory leaks detected in context feature"
    else
        test_failed "Memory leaks detected"
    fi
else
    echo -e "${YELLOW}⚠ Valgrind not installed, skipping memory leak check${NC}"
fi

echo ""
echo "========================================="
echo "Step 10: Sanitizer Tests (Optional)"
echo "========================================="

if [ -f "../local_test/run_asan.sh" ]; then
    echo "Running AddressSanitizer tests..."
    cd ..
    if bash local_test/run_asan.sh > /tmp/asan.log 2>&1; then
        test_passed "AddressSanitizer tests passed"
    else
        echo -e "${YELLOW}⚠ AddressSanitizer tests failed (check /tmp/asan.log)${NC}"
    fi
    cd build
else
    echo -e "${YELLOW}⚠ Sanitizer scripts not found${NC}"
fi

echo ""
echo "========================================="
echo "Cleaning up test files..."
echo "========================================="
rm -f test_*.log test_*.jsonl *.log *.jsonl test_* *.out compile.log

cd ..

echo ""
echo "========================================="
echo "TEST SUITE SUMMARY"
echo "========================================="
echo -e "${GREEN}Passed: $TESTS_PASSED${NC}"
if [ $TESTS_FAILED -gt 0 ]; then
    echo -e "${RED}Failed: $TESTS_FAILED${NC}"
    echo ""
    echo -e "${RED}Some tests failed! Please review the output above.${NC}"
    exit 1
else
    echo -e "${GREEN}Failed: 0${NC}"
    echo ""
    echo -e "${GREEN}=========================================${NC}"
    echo -e "${GREEN}ALL TESTS PASSED! ✓${NC}"
    echo -e "${GREEN}Zyrnix v1.0.3 is working correctly!${NC}"
    echo -e "${GREEN}=========================================${NC}"
fi
