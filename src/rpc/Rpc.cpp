//
//  Rpc.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 18/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Rpc.h"

using namespace gsl;

namespace brick
{
    
Rpc::Rpc(const char* ip, int port) {
    loop_ = (uv_loop_t*)malloc(sizeof(uv_loop_t));
    uv_loop_init(loop_);
    
    struct sockaddr_in addr;
    uv_ip4_addr(ip, port, &addr);
    
    uv_tcp_init(loop_, server_);
    uv_tcp_bind(server_, (const struct sockaddr*)&addr, 0);
}
  
void Rpc::connectionCb(uv_stream_t* server, int status) {
    auto self = (Rpc*)server->data;
    self->onNewConnection();
}
    
void Rpc::loop() {
    server_->data = this;
    uv_connection_cb cb = Rpc::connectionCb;
    uv_listen((uv_stream_t*)server_, 64, cb);
}
    
Rpc::~Rpc() {
    uv_loop_close(loop_);
    free(loop_);
}
    
}   // namespace brick
