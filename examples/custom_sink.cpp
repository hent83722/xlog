#include "xlog.hpp"
#include <iostream>

class CustomSink : public xlog::LogSink {
public:
    void log(const std::string& name, xlog::LogLevel level, const std::string& msg) override {
        std::cout << "[CUSTOM] " << msg << std::endl;
    }
};

int main() {
    auto logger = std::make_shared<xlog::Logger>("custom");
    logger->add_sink(std::make_shared<CustomSink>());
    logger->info("Custom sink log");
    return 0;
}
