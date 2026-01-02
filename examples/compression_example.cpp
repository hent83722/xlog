/**
 * @file compression_example.cpp
 * @brief Demonstrates log file compression features
 * 
 * This example shows:
 * 1. Compressed file sink with gzip
 * 2. Compressed file sink with zstd
 * 3. Compress-on-rotate functionality
 * 4. Compression statistics
 */

#include <Zyrnix/logger.hpp>
#include <Zyrnix/sinks/compressed_file_sink.hpp>
#include <iostream>
#include <string>

void example_gzip_compression() {
    std::cout << "\n=== Gzip Compression Example ===\n";
    
    // Create compressed file sink with gzip
    Zyrnix::CompressionOptions options;
    options.type = Zyrnix::CompressionType::Gzip;
    options.level = 6; // Default compression level (1-9)
    options.compress_on_rotate = true;
    
    auto sink = std::make_shared<Zyrnix::CompressedFileSink>(
        "logs/app.log",
        1024 * 1024, // Rotate at 1 MB
        5,           // Keep 5 rotated files
        options
    );
    
    auto logger = std::make_shared<Zyrnix::Logger>("app");
    logger->add_sink(sink);
    
    std::cout << "Writing logs with gzip compression...\n";
    
    // Write enough logs to trigger rotation
    for (int i = 0; i < 10000; ++i) {
        logger->info("Log message number " + std::to_string(i) + 
                     " with some additional data to increase file size");
    }
    
    logger->info("Compression test completed");
    
    auto stats = sink->get_compression_stats();
    std::cout << "\nCompression Statistics:\n";
    std::cout << "  Files compressed: " << stats.files_compressed << "\n";
    std::cout << "  Original size: " << stats.original_bytes << " bytes\n";
    std::cout << "  Compressed size: " << stats.compressed_bytes << " bytes\n";
    std::cout << "  Compression ratio: " << stats.compression_ratio << "x\n";
    std::cout << "  Space saved: " 
              << (100.0 * (stats.original_bytes - stats.compressed_bytes) / stats.original_bytes)
              << "%\n";
}

void example_zstd_compression() {
    std::cout << "\n=== Zstd Compression Example ===\n";
    
    // Zstd typically offers better compression ratios than gzip
    Zyrnix::CompressionOptions options;
    options.type = Zyrnix::CompressionType::Zstd;
    options.level = 3; // Default zstd level (1-22)
    options.compress_on_rotate = true;
    
    auto sink = std::make_shared<Zyrnix::CompressedFileSink>(
        "logs/app_zstd.log",
        1024 * 1024, // 1 MB
        3,           // Keep 3 files
        options
    );
    
    auto logger = std::make_shared<Zyrnix::Logger>("zstd_logger");
    logger->add_sink(sink);
    
    std::cout << "Writing logs with zstd compression...\n";
    
    for (int i = 0; i < 10000; ++i) {
        logger->info("Zstd compressed log entry " + std::to_string(i));
    }
    
    auto stats = sink->get_compression_stats();
    std::cout << "\nZstd Compression Statistics:\n";
    std::cout << "  Files compressed: " << stats.files_compressed << "\n";
    std::cout << "  Original size: " << stats.original_bytes << " bytes\n";
    std::cout << "  Compressed size: " << stats.compressed_bytes << " bytes\n";
    std::cout << "  Compression ratio: " << stats.compression_ratio << "x\n";
    std::cout << "  Space saved: " 
              << (100.0 * (stats.original_bytes - stats.compressed_bytes) / stats.original_bytes)
              << "%\n";
}

void example_compression_levels() {
    std::cout << "\n=== Compression Level Comparison ===\n";
    
    struct Result {
        int level;
        uint64_t compressed_size;
        double ratio;
    };
    
    std::vector<Result> results;
    
    // Test different compression levels
    for (int level : {1, 3, 6, 9}) {
        Zyrnix::CompressionOptions options;
        options.type = Zyrnix::CompressionType::Gzip;
        options.level = level;
        
        auto sink = std::make_shared<Zyrnix::CompressedFileSink>(
            "logs/test_level_" + std::to_string(level) + ".log",
            512 * 1024, // 512 KB
            1,
            options
        );
        
        auto logger = std::make_shared<Zyrnix::Logger>("test");
        logger->add_sink(sink);
        
        // Write test data
        for (int i = 0; i < 5000; ++i) {
            logger->info("Test message for compression level comparison " + std::to_string(i));
        }
        
        auto stats = sink->get_compression_stats();
        results.push_back({level, stats.compressed_bytes, stats.compression_ratio});
    }
    
    std::cout << "\nCompression Level Performance:\n";
    std::cout << "Level | Compressed Size | Ratio | Speed\n";
    std::cout << "------|----------------|-------|-------\n";
    for (const auto& r : results) {
        std::cout << "  " << r.level << "   | " 
                  << r.compressed_size << " bytes | "
                  << r.ratio << "x | "
                  << (r.level <= 3 ? "Fast" : r.level <= 6 ? "Medium" : "Slow")
                  << "\n";
    }
    
    std::cout << "\nRecommendations:\n";
    std::cout << "  Level 1-3: Best for high-throughput applications (fast)\n";
    std::cout << "  Level 6:   Balanced (default)\n";
    std::cout << "  Level 9:   Best compression for archival (slow)\n";
}

void example_production_usage() {
    std::cout << "\n=== Production Usage Example ===\n";
    
    Zyrnix::CompressionOptions options;
    options.type = Zyrnix::CompressionType::Gzip;
    options.level = 6;
    options.compress_on_rotate = true;
    
    // Production setup: 10 MB per file, keep 30 files
    // With 3:1 compression, this stores ~100 MB of logs in ~33 MB
    auto sink = std::make_shared<Zyrnix::CompressedFileSink>(
        "/var/log/myapp/app.log",
        10 * 1024 * 1024, // 10 MB
        30,               // 30 files
        options
    );
    
    auto logger = std::make_shared<Zyrnix::Logger>("production");
    logger->add_sink(sink);
    
    std::cout << "Production configuration:\n";
    std::cout << "  Max file size: 10 MB\n";
    std::cout << "  Max rotated files: 30\n";
    std::cout << "  Compression: gzip level 6\n";
    std::cout << "  Expected ratio: 3:1\n";
    std::cout << "  Total storage: ~100 MB logs in ~33 MB disk space\n";
    std::cout << "  Space savings: ~67%\n";
    
    logger->info("Production logging started with compression enabled");
}

int main() {
    std::cout << "Zyrnix Compression Examples\n";
    std::cout << "=========================\n";
    
    // Check availability
    std::cout << "Compression support:\n";
    std::cout << "  Gzip: " << (Zyrnix::CompressionUtils::is_gzip_available() ? "Available" : "Not available") << "\n";
    std::cout << "  Zstd: " << (Zyrnix::CompressionUtils::is_zstd_available() ? "Available" : "Not available") << "\n";
    
    example_gzip_compression();
    
    if (Zyrnix::CompressionUtils::is_zstd_available()) {
        example_zstd_compression();
    }
    
    example_compression_levels();
    example_production_usage();
    
    std::cout << "\n=== Key Benefits ===\n";
    std::cout << "1. Save 70-90% disk space on rotated log files\n";
    std::cout << "2. Automatic compression on rotation (no manual intervention)\n";
    std::cout << "3. Configurable compression levels (balance speed vs ratio)\n";
    std::cout << "4. Support for both gzip and zstd algorithms\n";
    std::cout << "5. Transparent operation (no changes to logging code)\n";
    
    return 0;
}
