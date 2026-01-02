#include "Zyrnix/async/thread_pool.hpp"
#include "Zyrnix/logger.hpp"
#include "Zyrnix/log_sink.hpp"
#include "Zyrnix/async/async_logger.hpp"

namespace Zyrnix {

ThreadPool::ThreadPool(size_t threads) : running(true) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] { worker(); });
    }
}

ThreadPool::~ThreadPool() {
    running = false;
    cv.notify_all();
    for (auto& t : workers) if (t.joinable()) t.join();
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push(std::move(task));
    }
    cv.notify_one();
}

void ThreadPool::worker() {
    while (running) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return !tasks.empty() || !running; });
            if (!running && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

}
