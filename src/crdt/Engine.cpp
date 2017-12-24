//
//  Engine.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Engine.h"

#include <iostream>
#include <algorithm>

using namespace gsl;
using namespace brick::detail;

namespace brick
{

namespace
{
    
Revision delta(const Revision& history, Revision& rev) {
    Expects(!(history.authorId() == rev.authorId() && history.revId() == rev.revId()));
    
    if (history.range().before(rev.range())) {
        switch (history.op()) {
            case Revision::Operation::insert:
                rev.range().offset(history.affectLength());
                break;
                
            case Revision::Operation::erase:
                rev.range().offset(-history.affectLength());
                break;
        }
    } else if (history.range().intersect(rev.range())) {
        switch (history.op()) {
            case Revision::Operation::insert: {
                switch (rev.op()) {
                    case Revision::Operation::insert:
                        rev.range().offset(history.affectLength());
                        break;
                        
                    case Revision::Operation::erase:
                        auto intersect = history.range().intersection(rev.range());
                        auto oldLength = rev.range().length;
                        rev.range().length = intersect.location - rev.range().location;
                        auto tail = Revision(rev.authorId(), rev.op(), Range(intersect.maxLocation(), oldLength - rev.range().length));
                        return tail;
                }
                break;
            }
                
            case Revision::Operation::erase: {
                switch (rev.op()) {
                    case Revision::Operation::insert: {
                        rev.range().location = history.range().location;
                        break;
                    }
                        
                    case Revision::Operation::erase:
                        if (history.range().contains(rev.range())) {
                            rev.setInvalid();
                        } else {
                            auto intersect = history.range().intersection(rev.range());
                            auto revMaxLoc = rev.range().maxLocation();
                            if (history.range().location <= rev.range().location) {
                                rev.range().location = history.range().location;
                                rev.range().length = std::max(0, revMaxLoc - intersect.maxLocation());
                            } else {
                                rev.range().length = history.range().location - rev.range().location;
                                auto tail = Revision(rev.authorId(), rev.op(), Range(intersect.maxLocation(), revMaxLoc - intersect.maxLocation()));
                                return tail;
                            }
                        }
                        break;
                }
                break;
            }
        }
    }
    return Revision();
}
    
}   // namespace
    
Engine::Engine(size_t authorId, not_null<Rope*> rope)
    : authorId_(authorId)
    , rope_(rope)
    , revisions_() {}
    
void Engine::insert(const CodePointList& cplist, int pos) {
    auto rev = Revision(authorId_, Revision::Operation::insert, Range(pos, 1), cplist);
    appendRevision(rev);
}
    
void Engine::erase(const Range& range) {
    auto rev = Revision(authorId_, Revision::Operation::erase, range);
    appendRevision(rev);
}
   
void Engine::appendRevision(Revision rev) {
    std::vector<Revision> additionals;
    for (auto& history : revisions_) {
        if (rev.authorId() == history.authorId()) {
            continue;
        }
        
        auto addi = delta(history, rev);
        if (addi.valid()) {
            additionals.push_back(addi);
        }
        
        if (!rev.valid()) {
            break;
        }
    }
    
    if (rev.valid()) {
        revisions_.push_back(rev);
        rev.apply(rope_);
    }
    
    for (const auto& additional : additionals) {
        additional.apply(rope_);
        revisions_.push_back(additional);
    }
}
    
void Engine::sync(size_t revId) {
    auto pos = std::find_if(revisions_.begin(), revisions_.end(), [revId](auto& rev) { return revId == rev.revId(); });
    if (pos != revisions_.end()) {
        std::advance(pos, 1);
        revisions_.erase(revisions_.begin(), pos);
    }
}
    
}   // namespace brick
