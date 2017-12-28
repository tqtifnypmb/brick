//
//  Client.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 28/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <string>

class Client {
public:
    Client(const char* ip, int16_t port);
    
    void write(const std::string& cnt);
    std::string read();
    
    int requestId() {
        return reqId_++;
    }
    
    ~Client();
    
private:    
    int server_;
    int reqId_;
};
