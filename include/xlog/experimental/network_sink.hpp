#pragma once
#include "log_sink.hpp"
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <asio.hpp>

namespace xlog {

class NetworkSink : public LogSink {
public:
    NetworkSink(const std::string& host, unsigned short port);
    ~NetworkSink();
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;

private:
    void worker();
    std::string host;
    unsigned short port;
    std::queue<std::string> queue;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread thread;
    std::atomic<bool> running;
};

}
