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

#include <memory>
#include <thread>
#include <gtest/gtest.h>

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
        
        rpc_ = std::make_unique<Rpc>("127.0.0.1", 10086, [](Rpc::RpcPeer* peer, Request req) {
            
        });
        
        loopThread_ = std::thread([this](){
            this->rpc_->loop();
        });
    }
    
    std::thread loopThread_;
    std::unique_ptr<Rpc> rpc_;
};
    
TEST_F(RpcTest, base) {
    auto req = Request(0, Request::MethodType::new_view, "");
    
    loopThread_.join();
}
    
}
