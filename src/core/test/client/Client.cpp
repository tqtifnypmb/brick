//
//  Client.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 28/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Client.h"

#include <gsl/gsl>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <algorithm>

Client::Client(const char* ip, int16_t port, read_cb user_input_cb, read_cb server_input_cb)
    : reqId_(0)
    , user_input_cb_(user_input_cb)
    , server_input_cb_(server_input_cb) {
        
    server_ = socket(PF_INET, SOCK_STREAM, 0);
    Expects(server_ >= 0);
    
    sockaddr_in sin;
    inet_pton(AF_INET, ip, &sin.sin_addr.s_addr);
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;
    
    int ret = connect(server_, (const struct sockaddr*)&sin, sizeof(sin));
    Expects(ret == 0);
    
    auto flags = fcntl(server_, F_GETFL, 0);
    flags = flags | O_NONBLOCK;
    fcntl(server_, F_SETFL, flags);
}

Client::~Client() {
    close(server_);
}

void Client::write(const std::string& cnt) {
    ssize_t remain = cnt.length();
    while (remain > 0) {
        remain -= ::write(server_, cnt.c_str() + cnt.length() - remain, remain);
    }
    if (remain < 0) {
        throw std::domain_error("Write error");
    }
}

std::string Client::read() {
    char buf[1024];
    ssize_t numRead = ::read(server_, buf, 1024);
    if (numRead == 0) {
        stopped_ = true;
        return "";
    } else if (numRead < 0) {
        throw std::domain_error("Read error");
    } else {
        return std::string(buf, numRead);
    }
}

int Client::loop() {
    for (;;) {
        fd_set freads;
        FD_ZERO(&freads);
        FD_SET(server_, &freads);
        FD_SET(STDIN_FILENO, &freads);
        
        auto max = std::max(server_, STDIN_FILENO) + 1;
        auto nready = select(max, &freads, nullptr, nullptr, 0);
        
        if (nready <= 0) {
            return nready;
        }
        
        if (FD_ISSET(STDIN_FILENO, &freads)) {
            user_input_cb_(*this);
        }
        
        if (FD_ISSET(server_, &freads)) {
            server_input_cb_(*this);
        }
        
        if (stopped_) {
            break;
        }
    }
    
    return 0;
}

void Client::stop() {
    stopped_ = true;
}
