#include "Zyrnix.hpp"

int main() {
    auto logger1 = Zyrnix::create_logger("logger1");
    auto logger2 = Zyrnix::create_logger("logger2");

    logger1->info("Logger1 message");
    logger2->warn("Logger2 warning");

    auto async_logger1 = Zyrnix::create_async_logger(logger1);
    async_logger1->log(Zyrnix::LogLevel::Debug, "Async message for logger1");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
