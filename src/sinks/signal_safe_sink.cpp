#include "xlog/sinks/signal_safe_sink.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>
#include <cerrno>

namespace xlog {

SignalSafeSink::SignalSafeSink(const std::string& path, size_t buffer_size) 
    : fd_(-1), capacity_(std::min(buffer_size, MAX_BUFFER_SIZE)) {
    fd_ = ::open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
}

SignalSafeSink::~SignalSafeSink() {
    if (fd_ >= 0) {
        flush();
        ::close(fd_);
        fd_ = -1;
    }
}

const char* SignalSafeSink::level_to_str(LogLevel level) {
   
    switch (level) {
        case LogLevel::Trace:    return "[TRACE] ";
        case LogLevel::Debug:    return "[DEBUG] ";
        case LogLevel::Info:     return "[INFO] ";
        case LogLevel::Warn:     return "[WARN] ";
        case LogLevel::Error:    return "[ERROR] ";
        case LogLevel::Critical: return "[CRITICAL] ";
        default:                 return "[UNKNOWN] ";
    }
}

void SignalSafeSink::log(const std::string& name, LogLevel level, const std::string& message) {
    if (fd_ < 0) {
        return;
    }
    

    const char* level_str = level_to_str(level);
    

    write_to_buffer(level_str, safe_strlen(level_str));
    

    write_to_buffer(message.c_str(), message.length());
    

    write_to_buffer("\n", 1);
    

    size_t write_pos = write_pos_.load(std::memory_order_relaxed);
    size_t read_pos = read_pos_.load(std::memory_order_relaxed);
    size_t current_size = write_pos - read_pos;
    if (current_size > capacity_ / 2) {
        flush_buffer();
    }
}

void SignalSafeSink::flush() {
    flush_buffer();
    if (fd_ >= 0) {

        ::fsync(fd_); 
    }
}

void SignalSafeSink::write_to_buffer(const char* data, size_t len) {
    if (len == 0) return;
    
    size_t write_pos, new_write_pos;
    size_t read_pos;
    
    do {
        write_pos = write_pos_.load(std::memory_order_relaxed);
        read_pos = read_pos_.load(std::memory_order_acquire);
        
        size_t available = capacity_ - (write_pos - read_pos);
        
        if (len > available) {
         
            bool expected = false;
            if (flush_in_progress_.compare_exchange_strong(expected, true, 
                    std::memory_order_acquire, std::memory_order_relaxed)) {
                flush_buffer();
                flush_in_progress_.store(false, std::memory_order_release);
            }
            
  
            write_pos = write_pos_.load(std::memory_order_relaxed);
            read_pos = read_pos_.load(std::memory_order_acquire);
            available = capacity_ - (write_pos - read_pos);
            
            if (len > available) {
        
                dropped_count_.fetch_add(1, std::memory_order_relaxed);
                return;
            }
        }
        
        new_write_pos = write_pos + len;
        
    } while (!write_pos_.compare_exchange_weak(write_pos, new_write_pos,
                std::memory_order_release, std::memory_order_relaxed));
    

    for (size_t i = 0; i < len; ++i) {
        size_t idx = (write_pos + i) % capacity_;
        buffer_[idx] = data[i];
    }
}

void SignalSafeSink::flush_buffer() {
    if (fd_ < 0) {
        return;
    }
    

    size_t write_pos = write_pos_.load(std::memory_order_acquire);
    size_t read_pos = read_pos_.load(std::memory_order_relaxed);
    
    if (write_pos == read_pos) {
        return; 
    }
    

    while (read_pos < write_pos) {
        size_t idx = read_pos % capacity_;
        size_t bytes_to_write;
        

        if (write_pos - read_pos <= capacity_ - idx) {
   
            bytes_to_write = write_pos - read_pos;
        } else {
      
            bytes_to_write = capacity_ - idx;
        }
        

        if (bytes_to_write > write_pos - read_pos) {
            bytes_to_write = write_pos - read_pos;
        }
        
        safe_write(fd_, &buffer_[idx], bytes_to_write);
        read_pos += bytes_to_write;
    }
    
 
    read_pos_.store(read_pos, std::memory_order_release);
}

void SignalSafeSink::safe_write(int fd, const char* data, size_t len) {
    
    size_t written = 0;
    while (written < len) {
        ssize_t ret = ::write(fd, data + written, len - written);
        if (ret < 0) {
            if (errno == EINTR) {

                continue;
            }

            break;
        }
        if (ret == 0) {
            
            break;
        }
        written += static_cast<size_t>(ret);
    }
}

size_t SignalSafeSink::safe_strlen(const char* str) {

    if (!str) return 0;
    
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void SignalSafeSink::safe_memcpy(char* dest, const char* src, size_t len) {
  
    for (size_t i = 0; i < len; ++i) {
        dest[i] = src[i];
    }
}

size_t SignalSafeSink::int_to_str(int value, char* buffer, size_t buffer_size) {
   
    if (buffer_size == 0) return 0;
    
    bool negative = value < 0;
    if (negative) value = -value;
    
  
    char temp[32];
    size_t pos = 0;
    
    do {
        if (pos >= sizeof(temp)) break;
        temp[pos++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    
    if (negative && pos < sizeof(temp)) {
        temp[pos++] = '-';
    }
    
    size_t out_pos = 0;
    while (pos > 0 && out_pos < buffer_size - 1) {
        buffer[out_pos++] = temp[--pos];
    }
    buffer[out_pos] = '\0';
    
    return out_pos;
}

} 
