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
#include "../rope/Range.h"
#include "../view/View.h"

namespace brick
{

class Core {
public:
    Core(const char* ip, int port);
    int run();
    
private:
    void handleReq(Rpc::RpcPeer* peer, Request req);
    void sendResp(Rpc::RpcPeer* client, Request req);
    void updateView(size_t viewId, const Engine::Delta& delta);
    
    View* viewWithId(size_t viewId);
    View* viewWithFilePath(const std::string& filePath);
    Rpc::RpcPeer* portForView(size_t viewId);
    
    size_t nextReqId_;
    size_t nextViewId_;
    std::map<size_t, Rpc::RpcPeer*> peersMap_;
    std::map<size_t, std::unique_ptr<View>> viewsMap_;
    std::unique_ptr<Rpc> rpc_;
};
    
}   // namespace brick
