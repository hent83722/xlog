#include "Zyrnix.hpp"

int main() {
    auto logger = std::make_shared<Zyrnix::Logger>("daily");
    auto sink = std::make_shared<Zyrnix::DailyFileSink>("daily_log");
    logger->add_sink(sink);

    logger->info("Logging daily message");
    return 0;
}
