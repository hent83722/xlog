#include "Zyrnix.hpp"

int main() {
    auto logger = Zyrnix::create_logger("basic");
    logger->trace("Trace message");
    logger->debug("Debug message");
    logger->info("Info message");
    logger->warn("Warning message");
    logger->error("Error message");
    logger->critical("Critical message");
    return 0;
}
