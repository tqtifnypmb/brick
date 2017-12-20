//
//  Rpc.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 18/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Rpc.h"

#include <string>
#include <iostream>

using namespace gsl;

namespace
{

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = (char*)malloc(suggested_size * sizeof(char));
    buf->len = suggested_size;
}
    
}

namespace brick
{
    
Rpc::Rpc(const char* ip, int port, std::function<void(RpcPeer*, Request)> msg_cb)
    : req_cb_(msg_cb) {
    loop_ = (uv_loop_t*)malloc(sizeof(uv_loop_t));
    uv_loop_init(loop_);
    
    struct sockaddr_in addr;
    uv_ip4_addr(ip, port, &addr);
    
    server_ = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop_, server_);
    uv_tcp_bind(server_, (const struct sockaddr*)&addr, 0);
}
  
void Rpc::onNewConnection(uv_tcp_t* client) {
    clients_.push_back(client);
    
    uv_read_start((uv_stream_t*)client, alloc_cb, Rpc::read_cb);
}
    
void Rpc::connection_cb(uv_stream_t* server, int status) {
    if (status < 0) {
        
    }
    
    auto self = (Rpc*)server->data;
    
    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(self->loop_, client);
    auto ret = uv_accept((uv_stream_t*)self->loop_, (uv_stream_t*)client);
    if (ret == 0) {
        self->onNewConnection(client);
    } else {
        
    }
}
    
void Rpc::onNewMsg(uv_stream_t* client, std::string msg) {
    auto req = Request::fromJson(msg);
    this->req_cb_((uv_handle_t*)client, req);
}
    
void Rpc::read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    auto self = (Rpc*)stream->data;
    
    if (nread < 0) {
        uv_read_stop(stream);
    } else {
        auto str = std::string(buf->base, buf->len);
        self->onNewMsg(stream, str);
    }
    
    if (buf->base != NULL) {
        free(buf->base);
    }
}
  
void Rpc::onHandleClosed(uv_handle_t* handle) {
    free(handle);
    
    std::lock_guard<std::mutex> lock(closeMutex_);
    
    auto ite = std::find(clients_.begin(), clients_.end(), (uv_tcp_t*)handle);
    if (ite != clients_.end()) {
        clients_.erase(ite);
    }
    
    --closeCount_;
    if (closeCount_ == 0) {
        closeCond_.notify_all();
    }
}
    
void Rpc::close_cb(uv_handle_t* h) {
    auto self = (Rpc*)h->data;
    self->onHandleClosed(h);
}
    
void Rpc::loop() {
    server_->data = this;
    uv_connection_cb cb = Rpc::connection_cb;
    uv_listen((uv_stream_t*)server_, 64, cb);
    uv_run(loop_, UV_RUN_DEFAULT);
}
    
void Rpc::close() {
    closeCount_ = clients_.size() + 1;
    for (auto client : clients_) {
        uv_close((uv_handle_t*)client, Rpc::close_cb);
    }
    
    uv_close((uv_handle_t*)server_, Rpc::close_cb);
    
    uv_loop_close(loop_);
    free(loop_);
}

void Rpc::closeAndWait() {
    close();
    
    auto mutex = std::unique_lock<std::mutex>(closeMutex_);
    closeCond_.wait(mutex);
}
   
void Rpc::write_cb(uv_write_t* req, int status) {
    // free write buffer
    uv_buf_t* buf = (uv_buf_t*)req->data;
    free(buf->base);
    free(buf);
}
    
void Rpc::send(Rpc* peer, const std::string& msg) {
    uv_buf_t* buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));
    buf->base = (char*)malloc(sizeof(char) * msg.length());
    strncpy(buf->base, msg.c_str(), msg.length());
    buf->len = msg.length();
    
    uv_write_t* writeReq = (uv_write_t*)malloc(sizeof(uv_write_t));
    writeReq->data = buf;
    uv_write(writeReq, (uv_stream_t*)peer, buf, 1, Rpc::write_cb);
}
    
Rpc::~Rpc() {
    closeAndWait();
}
    
}   // namespace brick
