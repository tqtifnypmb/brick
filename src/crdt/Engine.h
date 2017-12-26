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
#include "../types.h"

#include <gsl/gsl>
#include <vector>

namespace brick
{
   
class Engine {
public:
    Engine(size_t authorId, gsl::not_null<Rope*> rope);
    
    template <class Converter>
    void insert(gsl::span<const char> bytes, size_t pos);
    void insert(const detail::CodePointList& cplist, size_t pos);
    
    void erase(const Range& range);
    
    void appendRevision(Revision rev);
    
    void sync(size_t revId);
    
private:
    gsl::not_null<Rope*> rope_;
    std::vector<Revision> revisions_;
    size_t authorId_;
};
    
template <class Converter>
void Engine::insert(gsl::span<const char> bytes, size_t pos) {
    insert(Converter::encode(bytes), pos);
}
    
}   // namespace brick
