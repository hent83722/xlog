#pragma once
#include <string>

namespace xlog {

std::string trim(const std::string& s);

/**
 * @brief Platform path utilities (v1.1.2)
 * 
 * Provides cross-platform path handling with Windows Unicode support.
 */
namespace path {

/**
 * @brief Convert UTF-8 path to platform-native path
 * 
 * On Windows: Converts UTF-8 to UTF-16 (wstring)
 * On other platforms: Returns input unchanged
 * 
 * @param utf8_path Path encoded in UTF-8
 * @return Platform-native path string
 */
#ifdef _WIN32
std::wstring to_native(const std::string& utf8_path);
#else
inline const std::string& to_native(const std::string& utf8_path) { return utf8_path; }
#endif

/**
 * @brief Convert platform-native path to UTF-8
 * 
 * On Windows: Converts UTF-16 (wstring) to UTF-8
 * On other platforms: Returns input unchanged
 * 
 * @param native_path Platform-native path
 * @return UTF-8 encoded path string
 */
#ifdef _WIN32
std::string from_native(const std::wstring& native_path);
#else
inline const std::string& from_native(const std::string& native_path) { return native_path; }
#endif

/**
 * @brief Open a file with proper Unicode handling (v1.1.2)
 * 
 * Handles paths like "C:\Users\日本語\logs\app.log" correctly on Windows.
 * 
 * @param path UTF-8 encoded file path
 * @param mode File open mode (same as fopen)
 * @return FILE pointer or nullptr on error
 */
FILE* fopen_utf8(const std::string& path, const char* mode);

/**
 * @brief Check if file exists with Unicode path support
 * 
 * @param path UTF-8 encoded file path
 * @return true if file exists
 */
bool file_exists(const std::string& path);

/**
 * @brief Create directory with Unicode path support
 * 
 * @param path UTF-8 encoded directory path
 * @return true if directory was created or already exists
 */
bool create_directory(const std::string& path);

/**
 * @brief Rename file with Unicode path support
 * 
 * @param old_path UTF-8 encoded source path
 * @param new_path UTF-8 encoded destination path
 * @return true if rename succeeded
 */
bool rename_file(const std::string& old_path, const std::string& new_path);

/**
 * @brief Remove file with Unicode path support
 * 
 * @param path UTF-8 encoded file path
 * @return true if file was removed or didn't exist
 */
bool remove_file(const std::string& path);

} 

} 
