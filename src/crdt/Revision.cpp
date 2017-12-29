//
//  Revision.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Revision.h"
#include "../rope/Rope.h"
#include "../converter/Converter.h"

using namespace gsl;

namespace brick
{
        
Revision::Revision(size_t authorId, size_t revId, Operation op, const Range& range)
    : Revision(authorId, revId, op, range, {}) {}
    
Revision::Revision(size_t authorId, size_t revId, Operation op, const Range& range, const detail::CodePointList& cplist)
    : authorId_(authorId)
    , revId_(revId)
    , op_(op)
    , range_(range)
    , cplist_(cplist) {}
  
void Revision::apply(not_null<Rope*> rope) const {
    switch (op_) {
        case Operation::insert:
            rope->insert(cplist_, range_.location);
            break;
            
        case Operation::erase:
            rope->erase(range_);
            break;
    }
}
    
bool Revision::canApply(not_null<const Rope*> rope) const {
    auto validRange = Range(0, static_cast<int>(rope->size()));
    Range precondition;
    if (op_ == Revision::Operation::insert) {
        precondition = Range(range_.location, 0);
    } else {
        precondition = range_;
    }
    return validRange.contains(precondition);
}
    
}   // namespace brick
