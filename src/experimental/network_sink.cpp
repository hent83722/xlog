#include "xlog/experimental/network_sink.hpp"
#include "xlog/sinks/file_sink.hpp"
#include <asio.hpp>
#include <mutex>
#include <queue>
#include <string>

namespace xlog {

NetworkSink::NetworkSink(const std::string& h, unsigned short p)
    : host(h), port(p), running(true), thread(&NetworkSink::worker, this) {}

NetworkSink::~NetworkSink() {
    running = false;
    cv.notify_all();
    if (thread.joinable()) thread.join();
}

void NetworkSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    if (level < get_level()) return;
    std::lock_guard<std::mutex> lock(mtx);
    queue.push(formatter.format(logger_name, level, message));
    cv.notify_one();
}

void NetworkSink::worker() {
    asio::io_context io;
    asio::ip::tcp::socket socket(io);
    asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    asio::connect(socket, endpoints);
    while (running) {
        std::string msg;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return !queue.empty() || !running; });
            if (!running && queue.empty()) break;
            msg = std::move(queue.front());
            queue.pop();
        }
        asio::write(socket, asio::buffer(msg + "\n"));
    }
}

}
