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
    
Revision Engine::delta(const Revision& history, Revision& rev) {
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
                        
                    case Revision::Operation::erase: {
                        auto affectRange = Range(history.range().location, history.affectLength());
                        auto intersect = affectRange.intersection(rev.range());
                        auto oldLength = rev.range().length;
                        rev.range().length = std::max(intersect.location - rev.range().location, 0);
                        auto tail = Revision(rev.authorId(), nextRevId(), rev.op(), Range(affectRange.maxLocation(), oldLength - rev.range().length));
                        if (tail.valid()) {
                            if (rev.valid()) {  // tail have to apply after rev
                                tail.range().offset(-rev.range().length);
                            }
                            return tail;
                        }
                        break;
                    }
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
                                auto tail = Revision(rev.authorId(), nextRevId(), rev.op(), Range(intersect.maxLocation(), revMaxLoc - intersect.maxLocation()));
                                if (tail.valid()) {
                                    if (rev.valid()) {  // tail have to apply after rev
                                        tail.range().offset(-rev.range().length);
                                    }
                                }
                                
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
    
std::vector<Revision> Engine::delta(Revision& rev) {
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
        // rev has to apply first
        additionals.insert(additionals.begin(), rev);
    }
    return additionals;
}
    
Engine::Engine(size_t authorId, not_null<Rope*> rope)
    : authorId_(authorId)
    , rope_(rope)
    , revisions_() {}
    
void Engine::insert(const CodePointList& cplist, size_t pos) {
    auto rev = Revision(authorId_, nextRevId(), Revision::Operation::insert, Range(static_cast<int>(pos), 1), cplist);
    appendRevision(rev);
}
    
void Engine::erase(const Range& range) {
    auto rev = Revision(authorId_, nextRevId(), Revision::Operation::erase, range);
    appendRevision(rev);
}
   
bool Engine::appendRevision(Revision rev, bool pendingRev) {
    auto cache = rev;
    auto deltas = delta(rev);
    
    for (const auto& delta : deltas) {
        if (!delta.canApply(rope_)) {
            if (!pendingRev) {
                pendingRevs_.push_back(cache);
            }
            return false;
        }
    }
    
    for (const auto& delta : deltas) {
        delta.apply(rope_);
        revisions_.push_back(delta);
    }
    
    // return, if we're handling pending rev
    if (pendingRev) {
        return true;
    }
    
    auto iter = pendingRevs_.begin();
    while (iter != pendingRevs_.end()) {
        if (iter->canApply(rope_)) {
            auto rev = *iter;
            bool applied = appendRevision(rev, true);
            if (applied) {
                iter = pendingRevs_.erase(iter);
            } else {
                ++iter;
            }
        } else {
            ++iter;
        }
    }
    
    return true;
}
    
void Engine::appendRevision(Revision rev) {
    appendRevision(rev, false);
}
    
void Engine::sync(size_t revId) {
    auto pos = std::find_if(revisions_.begin(), revisions_.end(), [revId](auto& rev) { return revId == rev.revId(); });
    if (pos != revisions_.end()) {
        std::advance(pos, 1);
        revisions_.erase(revisions_.begin(), pos);
    }
}
    
}   // namespace brick
