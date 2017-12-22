//
//  Core.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 22/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <map>
#include <memory>

#include "../rpc/Request.h"
#include "../rpc/Rpc.h"

namespace brick
{

class View;
class Core {
public:
    Core(const char* ip, int port);
    int run();
    
private:
    void handleReq(Rpc::RpcPeer* peer, Request req);
    void sendResp(Rpc::RpcPeer* client, Request req);
    View* viewWithId(size_t viewId);
    Rpc::RpcPeer* clientWithId(size_t viewId);
    
    size_t nextViewId_;
    std::map<size_t, Rpc::RpcPeer*> clientsMap_;
    std::map<size_t, std::unique_ptr<View>> viewsMap_;
    std::unique_ptr<Rpc> rpc_;
};
    
}   // namespace brick
