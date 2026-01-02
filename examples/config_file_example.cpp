#include <Zyrnix/Zyrnix.hpp>
#include <Zyrnix/config.hpp>
#include <Zyrnix/logger.hpp>
#include <iostream>

/**
 * Example: Configuration File Support
 * 
 * Demonstrates how to load logger configurations from JSON files
 * instead of hardcoding them in your application.
 * 
 * Benefits:
 * - Change log levels and sinks without recompiling
 * - Different configs for dev, staging, production
 * - Easy A/B testing of logging strategies
 */

int main() {
    std::cout << "=== Zyrnix Configuration File Example ===" << std::endl;
    
    // Example 1: Create a JSON config file programmatically
    const char* config_json = R"({
        "loggers": [
            {
                "name": "app",
                "level": "debug",
                "async": false,
                "sinks": [
                    {"type": "stdout"},
                    {"type": "file", "path": "app.log"}
                ]
            },
            {
                "name": "network",
                "level": "info",
                "async": true,
                "sinks": [
                    {"type": "rotating", "path": "network.log", "max_size": 5242880, "max_files": 3}
                ]
            }
        ]
    })";
    
    // Example 2: Load from JSON string
    std::cout << "\n1. Loading configuration from JSON string..." << std::endl;
    if (Zyrnix::ConfigLoader::load_from_json_string(config_json)) {
        std::cout << "   ✓ Configuration loaded successfully" << std::endl;
    } else {
        std::cout << "   ✗ Failed to load configuration" << std::endl;
        return 1;
    }
    
    // Example 3: Create loggers from configuration
    std::cout << "\n2. Creating loggers from configuration..." << std::endl;
    auto loggers = Zyrnix::ConfigLoader::create_loggers();
    std::cout << "   ✓ Created " << loggers.size() << " loggers" << std::endl;
    
    // Example 4: Use the configured loggers
    std::cout << "\n3. Using configured loggers..." << std::endl;
    
    if (loggers.find("app") != loggers.end()) {
        auto app_logger = loggers["app"];
        app_logger->info("Application started");
        app_logger->debug("Debug information visible due to config");
        app_logger->warn("This is a warning message");
        std::cout << "   ✓ App logger working" << std::endl;
    }
    
    if (loggers.find("network") != loggers.end()) {
        auto network_logger = loggers["network"];
        network_logger->info("Network subsystem initialized");
        network_logger->debug("This won't appear (network logger is at Info level)");
        std::cout << "   ✓ Network logger working" << std::endl;
    }
    
    // Example 5: Load from file (typical production usage)
    std::cout << "\n4. Production usage - loading from file..." << std::endl;
    std::cout << "   Create a file named 'Zyrnix_config.json' with your configuration:" << std::endl;
    std::cout << "   {" << std::endl;
    std::cout << "     \"loggers\": [" << std::endl;
    std::cout << "       {" << std::endl;
    std::cout << "         \"name\": \"production\"," << std::endl;
    std::cout << "         \"level\": \"warn\"," << std::endl;
    std::cout << "         \"async\": true," << std::endl;
    std::cout << "         \"sinks\": [" << std::endl;
    std::cout << "           {\"type\": \"file\", \"path\": \"/var/log/app.log\"}," << std::endl;
    std::cout << "           {\"type\": \"rotating\", \"path\": \"app.log\", \"max_size\": 10485760, \"max_files\": 5}" << std::endl;
    std::cout << "         ]" << std::endl;
    std::cout << "       }" << std::endl;
    std::cout << "     ]" << std::endl;
    std::cout << "   }" << std::endl;
    std::cout << std::endl;
    std::cout << "   Then in your code:" << std::endl;
    std::cout << "   if (Zyrnix::ConfigLoader::load_from_json(\"Zyrnix_config.json\")) {" << std::endl;
    std::cout << "       auto loggers = Zyrnix::ConfigLoader::create_loggers();" << std::endl;
    std::cout << "       // Use loggers..." << std::endl;
    std::cout << "   }" << std::endl;
    
    // Example 6: Environment-specific configs
    std::cout << "\n5. Environment-specific configuration pattern..." << std::endl;
    std::cout << "   const char* env = std::getenv(\"ENVIRONMENT\");" << std::endl;
    std::cout << "   std::string config_file = std::string(\"config_\") + (env ? env : \"dev\") + \".json\";" << std::endl;
    std::cout << "   Zyrnix::ConfigLoader::load_from_json(config_file);" << std::endl;
    std::cout << "   // Automatically loads config_dev.json, config_prod.json, etc." << std::endl;
    
    std::cout << "\n=== Configuration example completed ===" << std::endl;
    std::cout << "Check app.log and network.log for output" << std::endl;
    
    return 0;
}
