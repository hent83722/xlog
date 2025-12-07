#pragma once
#include "../log_sink.hpp"
#include "../log_level.hpp"
#include <string>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace xlog {

class UdpSink : public LogSink {
public:
    UdpSink(const std::string& host, unsigned short port);
    ~UdpSink();
    void log(const std::string& logger_name, LogLevel level, const std::string& message) override;

private:
    int sockfd;
    struct ::sockaddr_storage dest;
    socklen_t dest_len;
    std::mutex mtx;
    bool initialized;
};

}
