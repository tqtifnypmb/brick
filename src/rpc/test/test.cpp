//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 19/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//


#include "../Request.h"
#include "../Rpc.h"
#include "../../3party/json.hpp"
#include "testClient/TestClient.h"
#include "testServer/testServer.h"

#include <memory>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>
#include <iostream>

using namespace brick;

namespace
{
    
TEST(Request, serialize_deserialize) {
    auto req = Request(0, Request::MethodType::new_view, "");
    EXPECT_EQ(req.id(), 0);
    EXPECT_EQ(req.params().dump(), "\"\"");
    EXPECT_EQ(req.method(), Request::MethodType::new_view);
    
    auto jstr = req.toJson();
    auto req2 = Request::fromJson(jstr);
    EXPECT_EQ(req.id(), req2.id());
    EXPECT_EQ(req.params().dump(), req2.params().dump());
    EXPECT_EQ(req.method(), req2.method());
    
    nlohmann::json params;
    params["viewId"] = 1;
    auto req3 = Request(1, Request::MethodType::close_view, params);
    EXPECT_EQ(req3.id(), 1);
    EXPECT_EQ(req3.params()["viewId"].get<size_t>(), 1);
    EXPECT_EQ(req3.method(), Request::MethodType::close_view);
}
    
class RpcTest: public ::testing::Test {
    protected:
    
    virtual void SetUp() {
        
        std::function<void(Rpc::RpcPeer*, Request)> onRequest = [this](Rpc::RpcPeer* peer, Request req) {
            for (auto& server : this->servers_) {
                if (server->client() == peer) {
                    server->handleRequest(req);
                    return;
                }
            }
            
            auto server = std::make_unique<TestServer>(this->rpc_.get(), peer);
            this->servers_.push_back(std::move(server));
            this->servers_.back()->handleRequest(req);
        };
        rpc_ = std::make_unique<Rpc>("127.0.0.1", 10086, onRequest);
        
        loopThread_ = std::thread([this](){
            this->rpc_->loop();
        });
                
        std::this_thread::sleep_for(std::chrono::seconds(1));
        client_ = std::make_unique<TestClient>("127.0.0.1", 10086);
    }
    
    virtual void TearDown() {
        auto exitReq = Request(1, Request::MethodType::exit);
        client_->sendOnly(exitReq);
        loopThread_.join();
    }
    
    std::vector<std::unique_ptr<TestServer>> servers_;
    std::unique_ptr<TestClient> client_;
    std::thread loopThread_;
    std::unique_ptr<Rpc> rpc_;
};
  
    
TEST_F(RpcTest, base) {
    auto newView = Request(0, Request::MethodType::new_view, "");

    auto respStr = client_->send(newView);
    auto resp = Request::fromJson(respStr);
    EXPECT_EQ(resp.id(), 0);
}
    
}
