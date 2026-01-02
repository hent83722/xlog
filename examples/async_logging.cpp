#include "Zyrnix.hpp"
#include <thread>

int main() {
    auto logger = Zyrnix::create_logger("async_example");
    auto async_logger = Zyrnix::create_async_logger(logger);

    for (int i = 0; i < 10; ++i) {
        async_logger->log(Zyrnix::LogLevel::Info, "Async log message " + std::to_string(i));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
