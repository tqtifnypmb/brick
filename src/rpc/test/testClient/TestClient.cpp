//
//  TestClient.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 20/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "TestClient.h"

#include <gsl/gsl>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

namespace brick
{
    
TestClient::TestClient(const char* ip, int16_t port) {
    server_ = socket(PF_INET, SOCK_STREAM, 0);
    Expects(server_ >= 0);
    
    sockaddr_in sin;
    inet_pton(AF_INET, ip, &sin.sin_addr.s_addr);
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;
    
    int ret = connect(server_, (const struct sockaddr*)&sin, sizeof(sin));
    Expects(ret == 0);
}
    
TestClient::~TestClient() {
    close(server_);
}
    
std::string TestClient::send(const Request& req) {
    write(req.toJson());
    return read();
}
    
void TestClient::sendOnly(const Request& req) {
    write(req.toJson());
}
    
std::string TestClient::read() {
    char buf[1024];
    ssize_t numRead = ::read(server_, buf, 1024);
    if (numRead <= 0) {
        throw std::domain_error("Read error");
    } else {
        return std::string(buf, numRead);
    }
}
    
void TestClient::write(const std::string& cnt) {
    ssize_t remain = cnt.length();
    while (remain > 0) {
        remain -= ::write(server_, cnt.c_str() + cnt.length() - remain, remain);
    }
    if (remain < 0) {
        throw std::domain_error("Write error");
    }
}
    
}
