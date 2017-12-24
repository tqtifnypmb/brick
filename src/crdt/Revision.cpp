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
    
namespace
{
    size_t nextRevId() {
        static size_t revId = 0;
        return revId++;
    }
}
    
Revision::Revision(size_t authorId, Operation op, const Range& range)
    : Revision(authorId, op, range, {}) {}
    
Revision::Revision(size_t authorId, Operation op, const Range& range, const detail::CodePointList& cplist)
    : authorId_(authorId)
    , revId_(nextRevId())
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
    
}   // namespace brick
