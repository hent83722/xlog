#include "Zyrnix.hpp"
#include <iostream>

class CustomSink : public Zyrnix::LogSink {
public:
    void log(const std::string& name, Zyrnix::LogLevel level, const std::string& msg) override {
        std::cout << "[CUSTOM] " << msg << std::endl;
    }
};

int main() {
    auto logger = std::make_shared<Zyrnix::Logger>("custom");
    logger->add_sink(std::make_shared<CustomSink>());
    logger->info("Custom sink log");
    return 0;
}
