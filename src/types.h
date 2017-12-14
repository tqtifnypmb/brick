//
//  types.h
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <array>
#include <memory>
#include <vector>

namespace brick
{
    
namespace detail
{
    using CodePoint = std::array<uint8_t, 4>;
    using CodePointList = std::vector<CodePoint>;
    using RopeNodePtr = std::shared_ptr<RopeNode>;
}
    
}
