//
//  Revision.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../rope/Range.h"

#include <gsl/gsl>
#include <vector>

namespace brick
{
    
class Revision {
public:
    enum class Operation {
        insert,
        erase,
    };
    
    Revision(size_t authorId, Operation op, const Range& range, gsl::span<const char*> text);
    Revision() = default;
    Revision(const Revision&) = default;
    
    size_t authorId() const {
        return authorId_;
    }
    
    Range range() const {
        return range_;
    }
    
    gsl::span<const char*> text() const {
        return text_;
    }
    
private:
        
    gsl::span<const char*> text_;
    size_t authorId_;
    Operation op_;
    Range range_;
};
    
}   // namespace brick
