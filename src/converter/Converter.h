//
//  Encoder.h
//  PretendToWork
//
//  Created by tqtifnypmb on 08/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../rope/RopeNode.h"

#include <string>

namespace brick
{

struct ASCIIConverter {
    typedef std::string result_type;
    
    static detail::CodePointList encode(const char* bytes, size_t len);
    static result_type decode(const detail::CodePointList& cplist);
};
    
}
