//
//  Revision.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Revision.h"

namespace brick
{
    
Revision::Revision(size_t authorId, Operation op, const Range& range, gsl::span<const char> text)
    : authorId_(authorId)
    , op_(op)
    , range_(range)
    , text_(text) {
        Expects(range.length == text.length());
    }
    
}   // namespace brick
