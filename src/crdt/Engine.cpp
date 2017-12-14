//
//  Engine.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Engine.h"

using namespace gsl;
using namespace details;

namespace brick
{

Engine::Engine(size_t authorId): authorId_(authorId), revisions_() {}
    
void Engine::insert(span<const char> bytes, size_t pos) {
    auto rev = Revision(authorId_, Revision::Operation::insert, Range(pos, bytes.length()), bytes);
    revisions_.push_back(rev);
}
    
void Engine::erase(const Range& range) {
    auto rev = Revision(authorId_, Revision::Operation::erase, range, nullptr);
    revisions_.push_back(rev);
}
    
void Engine::apply(not_null<Rope*> rope) {
    
}
    
} // namespace brick
