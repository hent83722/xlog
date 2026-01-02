#include "Zyrnix.hpp"

int main() {
    auto logger = Zyrnix::create_logger("levels");

    for (int i = 0; i <= static_cast<int>(Zyrnix::LogLevel::Critical); ++i) {
        logger->log(static_cast<Zyrnix::LogLevel>(i), "Test level " + std::to_string(i));
    }

    return 0;
}
