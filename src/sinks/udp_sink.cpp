#include "xlog/sinks/udp_sink.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>

namespace xlog {

UdpSink::UdpSink(const std::string& host, unsigned short port) : sockfd(-1), dest_len(0), initialized(false) {
    struct addrinfo hints;
    struct addrinfo* res = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    std::string port_str = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0) {
        return;
    }

    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        memcpy(&dest, p->ai_addr, p->ai_addrlen);
        dest_len = (socklen_t)p->ai_addrlen;
        initialized = true;
        break;
    }

    freeaddrinfo(res);
}

UdpSink::~UdpSink() {
    if (sockfd != -1) close(sockfd);
}

void UdpSink::log(const std::string& logger_name, LogLevel level, const std::string& message) {
    if (!initialized) return;
    std::lock_guard<std::mutex> lock(mtx);
    std::string out = logger_name.empty() ? message : (logger_name + ": " + message);

    out.push_back('\n');
    ssize_t sent = sendto(sockfd, out.data(), out.size(), 0, (struct sockaddr*)&dest, dest_len);
    (void)sent;
}

}
