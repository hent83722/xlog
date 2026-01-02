#pragma once
#include "Zyrnix_features.hpp"
#include <chrono>
#include <functional>
#include <atomic>
#include <mutex>

namespace Zyrnix {

class RateLimiter {
public:
    explicit RateLimiter(size_t messages_per_second = 0, size_t burst_capacity = 0);

    bool try_log();

    void reset();

    size_t available_tokens() const;

    uint64_t dropped_count() const { return dropped_count_.load(std::memory_order_relaxed); }

    bool is_enabled() const { return max_tokens_ > 0; }

private:
    void refill_tokens();

    size_t max_tokens_;                  
    size_t refill_rate_;                  
    std::atomic<double> tokens_;          
    std::chrono::steady_clock::time_point last_refill_;
    std::atomic<uint64_t> dropped_count_; 
    mutable std::mutex mutex_;
};

class SamplingLimiter {
public:
    explicit SamplingLimiter(size_t sample_rate = 1);

    bool should_log();

    void reset();

    uint64_t total_count() const { return counter_.load(std::memory_order_relaxed); }

    uint64_t dropped_count() const;

    bool is_enabled() const { return sample_rate_ > 1; }

private:
    size_t sample_rate_;
    std::atomic<uint64_t> counter_;
};

class CombinedLimiter {
public:
    explicit CombinedLimiter(
        size_t messages_per_second = 0,
        size_t burst_capacity = 0,
        size_t sample_rate = 1
    );

    bool should_log();

    struct Stats {
        uint64_t total_messages;
        uint64_t rate_limited_drops;
        uint64_t sampling_drops;
        uint64_t logged_messages;
    };

    Stats get_stats() const;
    void reset();

private:
    RateLimiter rate_limiter_;
    SamplingLimiter sampling_limiter_;
    std::atomic<uint64_t> logged_count_;
};

} // namespace Zyrnix
