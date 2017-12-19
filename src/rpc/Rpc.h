//
//  Rpc.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 18/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <memory>
#include <gsl/gsl>
#include <uv.h>

namespace brick
{
    
class Rpc {
public:
    
    Rpc(const char* ip, int port);
    ~Rpc();
    void loop();
    
private:
    
    void onNewConnection(uv_connect_t* req, int status);
    
    gsl::owner<uv_loop_t*> loop_;
    gsl::owner<uv_tcp_t*> server_;
};
    
}   // namespace brick
