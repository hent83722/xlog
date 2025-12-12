#include "xlog/rate_limiter.hpp"
#include <algorithm>

namespace xlog {

RateLimiter::RateLimiter(size_t messages_per_second, size_t burst_capacity)
    : max_tokens_(burst_capacity > 0 ? burst_capacity : messages_per_second)
    , refill_rate_(messages_per_second)
    , tokens_(static_cast<double>(max_tokens_))
    , last_refill_(std::chrono::steady_clock::now())
    , dropped_count_(0)
{
}

bool RateLimiter::try_log() {
    if (!is_enabled()) {
        return true;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    refill_tokens();

    double current = tokens_.load(std::memory_order_relaxed);
    if (current >= 1.0) {
        tokens_.store(current - 1.0, std::memory_order_relaxed);
        return true;
    }

    dropped_count_.fetch_add(1, std::memory_order_relaxed);
    return false;
}

void RateLimiter::refill_tokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refill_).count();
    
    if (elapsed > 0) {
        double tokens_to_add = (refill_rate_ * elapsed) / 1000.0;
        double current = tokens_.load(std::memory_order_relaxed);
        double new_tokens = std::min(current + tokens_to_add, static_cast<double>(max_tokens_));
        tokens_.store(new_tokens, std::memory_order_relaxed);
        last_refill_ = now;
    }
}

void RateLimiter::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_.store(static_cast<double>(max_tokens_), std::memory_order_relaxed);
    dropped_count_.store(0, std::memory_order_relaxed);
    last_refill_ = std::chrono::steady_clock::now();
}

size_t RateLimiter::available_tokens() const {
    return static_cast<size_t>(tokens_.load(std::memory_order_relaxed));
}

SamplingLimiter::SamplingLimiter(size_t sample_rate)
    : sample_rate_(sample_rate > 0 ? sample_rate : 1)
    , counter_(0)
{
}

bool SamplingLimiter::should_log() {
    if (!is_enabled()) {
        return true; // Sampling disabled
    }

    uint64_t count = counter_.fetch_add(1, std::memory_order_relaxed);
    return (count % sample_rate_) == 0;
}

void SamplingLimiter::reset() {
    counter_.store(0, std::memory_order_relaxed);
}

uint64_t SamplingLimiter::dropped_count() const {
    uint64_t total = counter_.load(std::memory_order_relaxed);
    if (!is_enabled() || total == 0) {
        return 0;
    }
    return total - (total / sample_rate_);
}

CombinedLimiter::CombinedLimiter(
    size_t messages_per_second,
    size_t burst_capacity,
    size_t sample_rate
)
    : rate_limiter_(messages_per_second, burst_capacity)
    , sampling_limiter_(sample_rate)
    , logged_count_(0)
{
}

bool CombinedLimiter::should_log() {
    if (!sampling_limiter_.should_log()) {
        return false;
    }

    if (!rate_limiter_.try_log()) {
        return false;
    }

    logged_count_.fetch_add(1, std::memory_order_relaxed);
    return true;
}

CombinedLimiter::Stats CombinedLimiter::get_stats() const {
    Stats stats;
    stats.total_messages = sampling_limiter_.total_count();
    stats.sampling_drops = sampling_limiter_.dropped_count();
    stats.rate_limited_drops = rate_limiter_.dropped_count();
    stats.logged_messages = logged_count_.load(std::memory_order_relaxed);
    return stats;
}

void CombinedLimiter::reset() {
    rate_limiter_.reset();
    sampling_limiter_.reset();
    logged_count_.store(0, std::memory_order_relaxed);
}

} // namespace xlog
