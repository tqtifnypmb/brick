//
//  CharTrait.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 05/01/2018.
//  Copyright © 2018 tqtifnypmb. All rights reserved.
//

#include "CharTrait.h"

namespace brick
{
 
bool ASCIICharTrait::isNewLine(const detail::CodePoint& cp) {
    auto ch = static_cast<uint8_t>(cp[0]);
    return ch == '\n';
}
    
size_t ASCIICharTrait::numOfLine(const detail::CodePointList& cplist) {
    size_t line = 0;
    for (auto& cp : cplist) {
        if (isNewLine(cp)) {
            line += 1;
        }
    }
    return line;
}
    
}
