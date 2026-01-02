#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/sinks/signal_safe_sink.hpp>
#include <Zyrnix/logger.hpp>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <memory>

/**
 * Example: Signal-Safe Logging for Crash Handlers
 * 
 * Demonstrates async-signal-safe logging for use in signal handlers
 * (SIGSEGV, SIGABRT, etc.) where most standard library functions are unsafe.
 * 
 * The SignalSafeSink uses only async-signal-safe POSIX functions:
 * - write() instead of fprintf()
 * - open() instead of fopen()
 * - No malloc/free
 * - No mutexes (lock-free ring buffer)
 */

// Global logger and sink for signal handler
std::shared_ptr<Zyrnix::Logger> g_crash_logger;
std::shared_ptr<Zyrnix::SignalSafeSink> g_crash_sink;

// Signal handler - must use only async-signal-safe functions
void crash_handler(int sig) {
    // This is safe because SignalSafeSink uses only async-signal-safe functions
    if (g_crash_logger) {
        switch (sig) {
            case SIGSEGV:
                g_crash_logger->log(Zyrnix::LogLevel::Critical, "Caught SIGSEGV (segmentation fault)");
                break;
            case SIGABRT:
                g_crash_logger->log(Zyrnix::LogLevel::Critical, "Caught SIGABRT (abort)");
                break;
            case SIGFPE:
                g_crash_logger->log(Zyrnix::LogLevel::Critical, "Caught SIGFPE (floating point exception)");
                break;
            case SIGILL:
                g_crash_logger->log(Zyrnix::LogLevel::Critical, "Caught SIGILL (illegal instruction)");
                break;
            default:
                g_crash_logger->log(Zyrnix::LogLevel::Critical, "Caught unknown signal");
                break;
        }
        
        // Flush to ensure crash log is written
        if (g_crash_sink) {
            g_crash_sink->flush();
        }
    }
    
    // Call default handler or exit
    signal(sig, SIG_DFL);
    raise(sig);
}

// Trigger a crash for demonstration
void trigger_crash(int type) {
    if (type == 1) {
        // Null pointer dereference -> SIGSEGV
        int* ptr = nullptr;
        *ptr = 42;
    } else if (type == 2) {
        // Abort -> SIGABRT
        abort();
    } else if (type == 3) {
        // Division by zero -> SIGFPE
        volatile int x = 0;
        volatile int y = 10 / x;
        (void)y;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== Zyrnix Signal-Safe Logging Example ===" << std::endl;
    std::cout << std::endl;
    
    // Create signal-safe sink
    std::cout << "1. Setting up signal-safe crash logger..." << std::endl;
    g_crash_sink = std::make_shared<Zyrnix::SignalSafeSink>("crash.log");
    
    if (!g_crash_sink->is_ready()) {
        std::cerr << "   ✗ Failed to create signal-safe sink" << std::endl;
        return 1;
    }
    std::cout << "   ✓ Signal-safe sink created" << std::endl;
    
    // Create logger
    g_crash_logger = std::make_shared<Zyrnix::Logger>("crash");
    g_crash_logger->add_sink(g_crash_sink);
    std::cout << "   ✓ Crash logger configured" << std::endl;
    
    // Install signal handlers
    std::cout << "\n2. Installing signal handlers..." << std::endl;
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
    signal(SIGFPE, crash_handler);
    signal(SIGILL, crash_handler);
    std::cout << "   ✓ Handlers installed for SIGSEGV, SIGABRT, SIGFPE, SIGILL" << std::endl;
    
    // Log normal operation
    std::cout << "\n3. Normal logging before crash..." << std::endl;
    g_crash_logger->log(Zyrnix::LogLevel::Info, "Application started normally");
    g_crash_logger->log(Zyrnix::LogLevel::Info, "All systems operational");
    std::cout << "   ✓ Normal logs written" << std::endl;
    
    // Choose crash type
    std::cout << "\n4. Crash simulation..." << std::endl;
    std::cout << "   To simulate a crash, run with argument:" << std::endl;
    std::cout << "   ./signal_safe_example 1  - Segmentation fault (null pointer)" << std::endl;
    std::cout << "   ./signal_safe_example 2  - Abort signal" << std::endl;
    std::cout << "   ./signal_safe_example 3  - Floating point exception" << std::endl;
    std::cout << std::endl;
    
    if (argc > 1) {
        int crash_type = atoi(argv[1]);
        std::cout << "   Triggering crash type " << crash_type << "..." << std::endl;
        std::cout << "   (The crash will be logged to crash.log)" << std::endl;
        sleep(1); // Give user time to see the message
        
        trigger_crash(crash_type);
        
        // This line should never be reached
        std::cout << "   ✗ Still alive (shouldn't happen!)" << std::endl;
    } else {
        std::cout << "   No crash triggered (safe mode)" << std::endl;
    }
    
    // Normal shutdown
    std::cout << "\n5. Normal shutdown..." << std::endl;
    g_crash_logger->log(Zyrnix::LogLevel::Info, "Application shutting down normally");
    g_crash_sink->flush();
    std::cout << "   ✓ Logs flushed successfully" << std::endl;
    
    std::cout << "\n=== Example completed ===" << std::endl;
    std::cout << "Check crash.log for logged messages" << std::endl;
    std::cout << "\nKey features of SignalSafeSink:" << std::endl;
    std::cout << "  • Uses only async-signal-safe POSIX functions" << std::endl;
    std::cout << "  • Lock-free ring buffer (no mutexes)" << std::endl;
    std::cout << "  • No malloc/free in signal handlers" << std::endl;
    std::cout << "  • Suitable for SIGSEGV, SIGABRT, SIGFPE handlers" << std::endl;
    
    return 0;
}
