//
//  Engine.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../rope/Rope.h"
#include "Revision.h"

#include <gsl/gsl>
#include <vector>

namespace brick
{
   
class Engine {
public:
    
    ///void insert(size_t pos, )
    void erase(const Rope::Range& range);
    
    void apply(gsl::not_null<Rope*> rope);
    
private:
    std::vector<Revision> revisions_;
    size_t authorId_;
};
    
}   // namespace brick
