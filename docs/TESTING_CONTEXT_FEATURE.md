# Testing the Log Context Feature (v1.0.3)

## Quick Start

### 1. Build the Project

```bash
cd /home/henri/Zyrnix
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

### 2. Compile the Context Example

```bash
cd build
g++ -std=c++17 -I../include -L. -o context_example ../examples/context_logging.cpp -lZyrnix -lpthread
```

### 3. Run the Example

```bash
./context_example
```

## What to Look For

The example demonstrates four key scenarios:

### 1. Payment Processing with Scoped Context
**File:** `payments.jsonl`

Every log automatically includes:
- `payment_id`: "pay-12345"
- `service`: "payment-processor"
- `amount`: "99.99" (added dynamically)
- `currency`: "USD" (added dynamically)

```bash
cat payments.jsonl | python3 -m json.tool
```

### 2. HTTP Request Tracking
**File:** `requests.jsonl`

All logs include:
- `request_id`: "req-abc123"
- `user_id`: "user-456"
- `endpoint`: "/api/v1/orders"

Nested contexts add:
- `operation`: "database_query" (only in nested scope)

```bash
cat requests.jsonl | python3 -m json.tool
```

### 3. Multi-threaded Isolation
**File:** `workers.jsonl`

Each thread has isolated context:
- Thread 0: `thread_id=0`, `worker_name=worker-0`
- Thread 1: `thread_id=1`, `worker_name=worker-1`
- Thread 2: `thread_id=2`, `worker_name=worker-2`

No cross-contamination between threads!

```bash
cat workers.jsonl | grep thread_id | python3 -m json.tool
```

### 4. Global Application Context
**File:** `app.jsonl`

All logs include global context:
- `app_version`: "1.0.3"
- `environment`: "production"
- `hostname`: "server-01"

Plus request-specific context when set:
- `request_id`: "req-789"

```bash
cat app.jsonl | python3 -m json.tool
```

## Write Your Own Test

Create a simple test file:

```cpp
#include <Zyrnix/structured_logger.hpp>
#include <Zyrnix/log_context.hpp>

int main() {
    auto logger = Zyrnix::StructuredLogger::create("test", "test.jsonl");
    
    // Test scoped context
    Zyrnix::ScopedContext ctx;
    ctx.set("user_id", "test-user-123");
    ctx.set("session_id", "session-xyz");
    
    logger->info("User logged in");
    logger->info("User viewed dashboard");
    logger->info("User logged out");
    
    return 0;
}
```

Compile and run:

```bash
cd build
g++ -std=c++17 -I../include -L. -o my_test my_test.cpp -lZyrnix -lpthread
./my_test
cat test.jsonl
```

Expected output: All three logs should include `user_id` and `session_id` fields automatically.

## Verify Context Features

### ✅ Automatic Field Injection
Context fields appear in all logs within scope without manual passing.

### ✅ Nested Contexts
Inner scopes add fields, outer scopes remain when inner scope exits.

### ✅ Thread Isolation
Each thread has its own context - no mixing.

### ✅ RAII Cleanup
Context automatically cleared when ScopedContext goes out of scope.

### ✅ Global Context
LogContext::set() creates application-wide fields in all logs.

## Clean Up

```bash
cd build
rm *.jsonl context_example
```

## Integration with Your Code

```cpp
#include <Zyrnix/log_context.hpp>
#include <Zyrnix/structured_logger.hpp>

void handle_request(const Request& req) {
    auto logger = Zyrnix::StructuredLogger::create("api", "api.jsonl");
    
    Zyrnix::ScopedContext ctx;
    ctx.set("request_id", req.id())
       .set("user_id", req.user_id())
       .set("ip", req.client_ip());
    
    logger->info("Request started");
    
    // All nested function calls automatically include context
    process_business_logic();
    
    logger->info("Request completed");
    
} // Context auto-cleaned here
```

All logs in `handle_request()` and any functions it calls will automatically include the context fields!
