#include "Zyrnix/util.hpp"
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace Zyrnix {

std::string trim(const std::string& s) {
    auto b = s.find_first_not_of(" \t\n\r");
    auto e = s.find_last_not_of(" \t\n\r");
    if (b == std::string::npos) return "";
    return s.substr(b, e - b + 1);
}

namespace path {

#ifdef _WIN32

std::wstring to_native(const std::string& utf8_path) {
    if (utf8_path.empty()) {
        return std::wstring();
    }
    

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_path.c_str(), 
                                          static_cast<int>(utf8_path.size()), nullptr, 0);
    if (size_needed <= 0) {
        return std::wstring();
    }
    
    std::wstring result(size_needed, L'\0');
    int converted = MultiByteToWideChar(CP_UTF8, 0, utf8_path.c_str(), 
                                        static_cast<int>(utf8_path.size()), 
                                        &result[0], size_needed);
    if (converted <= 0) {
        return std::wstring();
    }
    
    return result;
}

std::string from_native(const std::wstring& native_path) {
    if (native_path.empty()) {
        return std::string();
    }
    

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, native_path.c_str(),
                                          static_cast<int>(native_path.size()),
                                          nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) {
        return std::string();
    }
    
    std::string result(size_needed, '\0');
    int converted = WideCharToMultiByte(CP_UTF8, 0, native_path.c_str(),
                                        static_cast<int>(native_path.size()),
                                        &result[0], size_needed, nullptr, nullptr);
    if (converted <= 0) {
        return std::string();
    }
    
    return result;
}

FILE* fopen_utf8(const std::string& path, const char* mode) {
    std::wstring wpath = to_native(path);
    if (wpath.empty() && !path.empty()) {
        return nullptr;
    }
    

    std::wstring wmode;
    while (*mode) {
        wmode += static_cast<wchar_t>(*mode++);
    }
    
    return _wfopen(wpath.c_str(), wmode.c_str());
}

bool file_exists(const std::string& path) {
    std::wstring wpath = to_native(path);
    if (wpath.empty() && !path.empty()) {
        return false;
    }
    
    DWORD attrs = GetFileAttributesW(wpath.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

bool create_directory(const std::string& path) {
    std::wstring wpath = to_native(path);
    if (wpath.empty() && !path.empty()) {
        return false;
    }
    
    if (CreateDirectoryW(wpath.c_str(), nullptr)) {
        return true;
    }
    

    DWORD err = GetLastError();
    return (err == ERROR_ALREADY_EXISTS);
}

bool rename_file(const std::string& old_path, const std::string& new_path) {
    std::wstring wold = to_native(old_path);
    std::wstring wnew = to_native(new_path);
    
    if ((wold.empty() && !old_path.empty()) || (wnew.empty() && !new_path.empty())) {
        return false;
    }
    
    return MoveFileExW(wold.c_str(), wnew.c_str(), 
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != 0;
}

bool remove_file(const std::string& path) {
    std::wstring wpath = to_native(path);
    if (wpath.empty() && !path.empty()) {
        return false;
    }
    
    if (DeleteFileW(wpath.c_str())) {
        return true;
    }
    

    DWORD err = GetLastError();
    return (err == ERROR_FILE_NOT_FOUND);
}

#else 

FILE* fopen_utf8(const std::string& path, const char* mode) {

    return std::fopen(path.c_str(), mode);
}

bool file_exists(const std::string& path) {
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
}

bool create_directory(const std::string& path) {
    if (mkdir(path.c_str(), 0755) == 0) {
        return true;
    }
    // Check if already exists
    return (errno == EEXIST);
}

bool rename_file(const std::string& old_path, const std::string& new_path) {
    return std::rename(old_path.c_str(), new_path.c_str()) == 0;
}

bool remove_file(const std::string& path) {
    if (std::remove(path.c_str()) == 0) {
        return true;
    }

    return (errno == ENOENT);
}

#endif 

} 

} 
