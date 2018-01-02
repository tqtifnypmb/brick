//
//  Rpc.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 18/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <memory>
#include <vector>

#include <gsl/gsl>
#include <uv.h>

#include "Request.h"

namespace brick
{
    
class Rpc {
public:
    using RpcPeer = uv_handle_t;
    
    enum class LoopState: int {
        looping = 0,
        closing = 1,
        closed,
        ready
    };
    
    Rpc(const char* ip, int port, const std::function<void(RpcPeer*, Request)>& req_cb);
    Rpc(const Rpc&) = delete;
    Rpc& operator=(const Rpc&) = delete;
    ~Rpc();
    
    void send(RpcPeer* peer, const std::string& msg);
    void loop();
    void close();
    void close(RpcPeer* peer);
    
private:
    
    void onNewConnection(uv_tcp_t* client);
    void onNewMsg(uv_stream_t* client, std::string msg);
    void onHandleClosed(uv_handle_t* handle);
    
    static void connection_cb(uv_stream_t* server, int status);
    static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    static void close_cb(uv_handle_t*);
    static void write_cb(uv_write_t* req, int status);

    LoopState state_;

    std::function<void(RpcPeer* peer, Request)> req_cb_;
    std::vector<uv_tcp_t*> clients_;
    gsl::owner<uv_loop_t*> loop_;
    gsl::owner<uv_tcp_t*> server_;
};
    
}   // namespace brick
