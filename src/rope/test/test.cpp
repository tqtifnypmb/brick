//
//  test.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 09/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "../Rope.h"
#include "../../converter/Converter.h"

#include <iostream>
#include <cassert>

using namespace brick;

int main() {
//    auto cplist = ASCIIConverter::encode("abcdef", 6);
//    auto str = ASCIIConverter::decode(cplist);
//    assert(str == "abcdef");
   
    auto rope = Rope();
    auto str = std::string("abcdefhijklmnopqrstuvwxyz");
    rope.insert<ASCIIConverter>(str.c_str(), str.length(), 0);
    std::cout<<rope.string()<<std::endl;
    
    auto num = std::string("1234567890");
    rope.insert<ASCIIConverter>(num.c_str(), num.length(), str.length() - 1);
    std::cout<<rope.string()<<std::endl;
    return 0;
}
