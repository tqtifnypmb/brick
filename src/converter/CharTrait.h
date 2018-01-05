//
//  CharTrait.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 05/01/2018.
//  Copyright © 2018 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../types.h"

namespace brick
{
    
struct ASCIICharTrait {
    static bool isNewLine(const detail::CodePoint& cp);
    static size_t numOfLine(const detail::CodePointList& cplist);
};
    
}
