//
//  Client.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 28/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <string>
#include <functional>

class Client {
public:
    using read_cb = std::function<void(Client&)>;
    
    Client(const char* ip, int16_t port, read_cb user_input_cb, read_cb server_input_cb);
    
    void write(const std::string& cnt);
    std::string read();
    
    int requestId() {
        return reqId_++;
    }
    
    int loop();
    void stop();
    ~Client();
    
private:
    read_cb user_input_cb_;
    read_cb server_input_cb_;
    int server_;
    int reqId_;
    bool stopped_;
};
