cmake_minimum_required(VERSION 3.16)
project(XLogSimpleDemo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find threads
find_package(Threads REQUIRED)

# XLog library path
set(XLOG_DIR "${CMAKE_SOURCE_DIR}/../xlog")
set(XLOG_INCLUDE_DIRS 
    "${XLOG_DIR}/include"
    "${XLOG_DIR}/include/xlog"
)
set(XLOG_LIBRARY "${XLOG_DIR}/build/libxlog.a")

# Include directories
include_directories(${XLOG_INCLUDE_DIRS})

# Create simple demo executable
add_executable(xlog_demo src/simple_demo.cpp)

# Link libraries
target_link_libraries(xlog_demo 
    ${XLOG_LIBRARY}
    Threads::Threads
)

# Compiler options
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(xlog_demo PRIVATE NDEBUG)
    target_compile_options(xlog_demo PRIVATE -O3)
else()
    target_compile_options(xlog_demo PRIVATE -g -O0)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "XLog library: ${XLOG_LIBRARY}")
