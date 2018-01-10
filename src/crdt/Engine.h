//
//  Engine.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../rope/Rope.h"
#include "../types.h"
#include "Revision.h"

#include <gsl/gsl>
#include <vector>
#include <map>

namespace brick
{
   
class Engine {
public:
    using DeltaList = std::vector<Revision>;
    
    Engine(size_t authorId, gsl::not_null<Rope*> rope);
    
    template <class Converter>
    DeltaList insert(gsl::span<const char> bytes, size_t pos);
    DeltaList insert(const detail::CodePointList& cplist, size_t pos);
    
    DeltaList erase(const Range& range);
    
    DeltaList appendRevision(Revision rev);
    
    DeltaList sync(const Engine& other);
    
    const std::vector<Revision>& revisions() const {
        return revisions_;
    }
    
    std::vector<Revision>& revisions() {
        return revisions_;
    }
    
    size_t authorId() const {
        return authorId_;
    }
    
    size_t nextRevId() {
        return revId_++;
    }
    
private:
    Revision delta(const Revision& history, Revision& rev);
    std::vector<Revision> delta(Revision& rev);
    
    bool appendRevision(Revision rev, bool pendingRev, std::vector<Revision>* deltas);
    
    gsl::not_null<Rope*> rope_;
    std::vector<Revision> revisions_;
    std::vector<Revision> pendingRevs_;
    std::map<size_t, size_t> syncState_;    // authorId_ -> next valid revId
    size_t authorId_;
    size_t revId_;
};
    
template <class Converter>
Engine::DeltaList Engine::insert(gsl::span<const char> bytes, size_t pos) {
    return insert(Converter::encode(bytes), pos);
}
    
}   // namespace brick
