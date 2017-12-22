//
//  main.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 18/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Core.h"

#include <string>

using namespace brick;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        throw std::invalid_argument("Please specify a address");
    }
    
    const char* ip = argv[1];
    const char* portStr = argv[2];
    int port = std::stoi(std::string(portStr, strnlen(portStr, 56)));
    
    auto core = Core(ip, port);
    return core.run();
}
