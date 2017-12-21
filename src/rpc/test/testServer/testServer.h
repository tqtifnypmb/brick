//
//  testSever.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 21/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../../Rpc.h"
#include "../../Request.h"

#include <vector>

namespace brick
{
    
class TestServer {
public:
    
    TestServer(Rpc* rpc, Rpc::RpcPeer* c);
    
    void handleRequest(const Request& req);
    
    Rpc::RpcPeer* client() const {
        return client_;
    }
    
    const std::vector<size_t>& viewIds() {
        return viewIds_;
    }
    
    size_t viewId() {
        return viewId_;
    }
    
private:
    
    void sendResp(const Request& resp);
    
    size_t viewId_;
    std::vector<size_t> viewIds_;
    Rpc* rpc_;
    Rpc::RpcPeer* client_;
};
    
}   // namespace brick
