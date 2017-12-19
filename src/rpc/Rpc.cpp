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
  
void Rpc::onNewConnection(uv_connect_t* req, int status) {
        
}
    
void Rpc::loop() {
    //uv_listen(server_, 64, uv_connection_cb cb)
}
    
Rpc::~Rpc() {
    uv_loop_close(loop_);
    free(loop_);
}
    
}   // namespace brick
