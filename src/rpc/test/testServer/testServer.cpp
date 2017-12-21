//
//  testSever.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 21/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "testServer.h"
#include "../../../3party/json.hpp"
#include <gsl/gsl>
#include <iostream>

namespace brick
{
    
TestServer::TestServer(Rpc* rpc, Rpc::RpcPeer* c)
    : rpc_(rpc)
    , client_(c)
    , viewId_(0) {}
    
void TestServer::handleRequest(const Request& req) {
    switch (req.method()) {
        case Request::MethodType::new_view: {
            auto viewId = viewId_++;
            viewIds_.push_back(viewId);
            auto params = nlohmann::json::object();
            params["viewId"] = viewId;
            auto resp = req.response(params);
            sendResp(resp);
            break;
        }
            
        case Request::MethodType::close_view: {
            auto viewId = req.params()["viewId"].get<size_t>();
            auto iter = std::find(viewIds_.begin(), viewIds_.end(), viewId);
            Expects(iter != viewIds_.end());
            viewIds_.erase(iter);
            break;
        }
            
        case Request::MethodType::text: {
            break;
        }
            
        case Request::MethodType::exit: {
            rpc_->closeAndWait();
            break;
        }
            
        case Request::MethodType::none: {
            throw std::invalid_argument("Response sent to server");
            break;
        }
    }
}
    
void TestServer::sendResp(const Request& resp) {
    rpc_->send(client_, resp.toJson());
}
    
}
