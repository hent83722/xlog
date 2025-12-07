cmake_minimum_required(VERSION 3.16)

find_package(Threads REQUIRED)
find_package(fmt REQUIRED)

file(GLOB TEST_SOURCES "*.cpp")

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE xlog fmt::fmt Threads::Threads)
enable_testing()
add_test(NAME xlog_tests COMMAND tests)

option(ENABLE_FUZZ "Build fuzz targets" OFF)
if(ENABLE_FUZZ)
	add_executable(fuzz_formatter fuzz_formatter.cpp)
	target_link_libraries(fuzz_formatter PRIVATE xlog fmt::fmt Threads::Threads)
	# Recommended flags for libFuzzer + address sanitizer for CI fuzz runs
	target_compile_options(fuzz_formatter PRIVATE -g -O1 -fsanitize=address,fuzzer-no-link)
	target_link_options(fuzz_formatter PRIVATE -fsanitize=address,fuzzer)
endif()
