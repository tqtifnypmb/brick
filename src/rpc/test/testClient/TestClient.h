//
//  TestClient.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 20/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <string>

#include "../../Request.h"

namespace brick
{
    
class TestClient {
public:
    TestClient(const char* ip, int16_t port);
    ~TestClient();
    std::string send(const Request& req);
    void sendOnly(const Request& req);
    
private:
    std::string read();
    void write(const std::string& cnt);
    
    int server_;
};
    
}   // namespace brick
