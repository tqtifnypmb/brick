//
//  Revision.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../rope/Range.h"
#include "../types.h"

#include <gsl/gsl>
#include <vector>
#include <string>

namespace brick
{
    
class Rope;
class Revision {
public:
    enum class Operation {
        insert,
        erase,
    };
    
    Revision(size_t authorId, size_t revId, Operation op, const Range& range);
    Revision(size_t authorId, size_t revId, Operation op, const Range& range, const detail::CodePointList& text);
    Revision(const Revision&) = default;
    Revision() = default;
    
    void apply(gsl::not_null<Rope*> rope) const;
    bool canApply(gsl::not_null<const Rope*> rope) const;
    
    size_t authorId() const {
        return authorId_;
    }
    
    bool prior(const Revision& rev) const {
        return authorId() <= rev.authorId();
    }
    
    size_t revId() const {
        return revId_;
    }
    
    const Range& range() const {
        return range_;
    }
    
    Range& range() {
        return range_;
    }
    
    int affectLength() const {
        if (op() == Operation::insert) {
            return static_cast<int>(cplist().size());
        } else {
            return range().length;
        }
    }
    
    Operation op() const {
        return op_;
    }
    
    bool valid() const {
        return !range_.empty() && range_.location >= 0;
    }
    
    void setInvalid() {
        range_.length = 0;
    }
    
    const detail::CodePointList& cplist() const {
        return cplist_;
    }
    
private:
    
    size_t authorId_;
    
    // revId_ only identify individual revision
    // newer revision doesn't have to have greater revId_
    size_t revId_;
    detail::CodePointList cplist_;
    Operation op_;
    Range range_;
};
    
}   // namespace brick
