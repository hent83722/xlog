#pragma once
#include "../log_record.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

namespace Zyrnix {

/**
 * @brief Thread-safe async queue with flush guarantees
 * 
 * v1.1.2: Added shutdown timeout and drain guarantees
 */
class AsyncQueue {
public:
    /**
     * @brief Construct async queue
     * @param shutdown_timeout_ms Maximum time to wait for queue drain on shutdown (default 5000ms)
     */
    explicit AsyncQueue(size_t shutdown_timeout_ms = 5000);
    
    /**
     * @brief Destructor - waits for queue to drain with timeout
     */
    ~AsyncQueue();
    
    /**
     * @brief Push a log record to the queue
     * @param record The log record to push
     * @return true if pushed successfully, false if queue is shutting down
     */
    bool push(LogRecord&& record);
    
    /**
     * @brief Pop a log record from the queue (blocking)
     * @param record Output parameter for the popped record
     * @return true if a record was popped, false if queue is shutting down
     */
    bool pop(LogRecord& record);
    
    /**
     * @brief Check if queue is empty
     */
    bool empty() const;
    
    /**
     * @brief Get current queue size
     */
    size_t size() const;
    
    /**
     * @brief Initiate graceful shutdown
     * @param wait_for_drain If true, blocks until queue is drained or timeout
     * @return true if queue was fully drained, false if timeout occurred
     */
    bool shutdown(bool wait_for_drain = true);
    
    /**
     * @brief Check if shutdown has been initiated
     */
    bool is_shutting_down() const;
    
    /**
     * @brief Set shutdown timeout
     * @param timeout_ms Timeout in milliseconds
     */
    void set_shutdown_timeout(size_t timeout_ms);
    
    /**
     * @brief Get number of messages dropped during shutdown timeout
     */
    size_t dropped_on_shutdown() const;

private:
    std::queue<LogRecord> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::condition_variable drain_cv_;
    std::atomic<bool> shutdown_{false};
    std::atomic<size_t> dropped_count_{0};
    size_t shutdown_timeout_ms_;
};

}
