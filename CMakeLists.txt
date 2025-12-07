cmake_minimum_required(VERSION 3.16)
project(xlog VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB XLOG_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sinks/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/experimental/*.cpp"
)

add_library(xlog STATIC ${XLOG_SOURCES})

target_include_directories(xlog PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/xlog>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS xlog
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/xlogConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/xlogConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/xlogConfig.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/xlogConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/xlogConfigVersion.cmake"
    DESTINATION lib/cmake/xlog
)
