#include "xlog/async/async_queue.hpp"
#include "xlog/logger.hpp"
#include "xlog/log_sink.hpp"
#include "xlog/formatter.hpp"

namespace xlog {

AsyncQueue::AsyncQueue(size_t shutdown_timeout_ms)
    : shutdown_timeout_ms_(shutdown_timeout_ms) {
}

AsyncQueue::~AsyncQueue() {
    shutdown(true);
}

bool AsyncQueue::push(LogRecord&& record) {
    if (shutdown_.load(std::memory_order_acquire)) {
        return false;
    }
    
    {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(std::move(record));
    }
    cv_.notify_one();
    return true;
}

bool AsyncQueue::pop(LogRecord& record) {
    std::unique_lock<std::mutex> lock(mtx_);
    
    cv_.wait(lock, [this] {
        return !queue_.empty() || shutdown_.load(std::memory_order_acquire);
    });
    
    if (queue_.empty()) {
        return false;
    }
    
    record = std::move(queue_.front());
    queue_.pop();
    

    if (queue_.empty()) {
        drain_cv_.notify_all();
    }
    
    return true;
}

bool AsyncQueue::empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.empty();
}

size_t AsyncQueue::size() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.size();
}

bool AsyncQueue::shutdown(bool wait_for_drain) {
    shutdown_.store(true, std::memory_order_release);
    cv_.notify_all();
    
    if (!wait_for_drain) {
        return empty();
    }
    
    std::unique_lock<std::mutex> lock(mtx_);
    
    bool drained = drain_cv_.wait_for(lock, 
        std::chrono::milliseconds(shutdown_timeout_ms_),
        [this] { return queue_.empty(); }
    );
    
    if (!drained) {
        dropped_count_.store(queue_.size(), std::memory_order_release);
        while (!queue_.empty()) {
            queue_.pop();
        }
    }
    
    return drained;
}

bool AsyncQueue::is_shutting_down() const {
    return shutdown_.load(std::memory_order_acquire);
}

void AsyncQueue::set_shutdown_timeout(size_t timeout_ms) {
    shutdown_timeout_ms_ = timeout_ms;
}

size_t AsyncQueue::dropped_on_shutdown() const {
    return dropped_count_.load(std::memory_order_acquire);
}

} // namespace xlog
