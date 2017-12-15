//
//  Engine.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Engine.h"

#include <iostream>

using namespace gsl;
using namespace details;

namespace brick
{

namespace
{
    
Revision delta(Revision& history, Revision& rev) {
    Expects(!(history.authorId() == rev.authorId() && history.revId() == rev.revId()));
    
    if (history.range().before(rev.range()) && history.prior(rev)) {
        switch (history.op()) {
            case Revision::Operation::insert:
                rev.range().offset(history.length());
                break;
                
            case Revision::Operation::erase:
                rev.range().offset(-history.length());
                break;
        }
    } else if (rev.range().before(history.range()) && rev.prior(history)) {
        switch (rev.op()) {
            case Revision::Operation::insert:
                history.range().offset(rev.length());
                break;
                
            case Revision::Operation::erase:
                history.range().offset(-rev.length());
                break;
        }
    } else if (history.range().intersect(rev.range())) {
        switch (history.op()) {
            case Revision::Operation::insert: {
                switch (rev.op()) {
                    case Revision::Operation::insert:
                        if (history.prior(rev)) {
                            rev.range().offset(history.length());
                        } else {
                            history.range().offset(rev.length());
                        }
                        break;
                        
                    case Revision::Operation::erase:
                        if (rev.prior(history)) {
                            history.range().location = rev.range().location;
                        } else {
                            auto intersect = history.range().intersection(rev.range());
                            auto oldLength = rev.range().length;
                            rev.range().length = intersect.location - rev.range().location;
                            auto tail = Revision(rev.authorId(), rev.op(), Range(intersect.maxLocation(), oldLength - rev.range().length), nullptr);
                            return tail;
                        }
                        break;
                }
                break;
            }
                
            case Revision::Operation::erase: {
                switch (rev.op()) {
                    case Revision::Operation::insert: {
                        if (history.prior(rev)) {
                            rev.range().location = history.range().location;
                        } else {
                            auto intersect = history.range().intersection(rev.range());
                            auto oldLength = history.range().length;
                            history.range().length = intersect.location - history.range().location;
                            auto tail = Revision(history.authorId(), history.op(), Range(intersect.maxLocation(), oldLength - history.range().length), nullptr);
                            return tail;
                        }
                        break;
                    }
                        
                    case Revision::Operation::erase:
                        auto intersect = history.range().intersection(rev.range());
                        if (history.range().location <= rev.range().location) {
                            history.range().length += rev.range().length - intersect.length;
                            rev.range().length = 0;
                        } else {
                            history.range().location = rev.range().location;
                            rev.range().length = 0;
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
    
Engine::Engine(size_t authorId): authorId_(authorId), revisions_() {}
    
void Engine::insert(span<const char> bytes, size_t pos) {
    auto rev = Revision(authorId_, Revision::Operation::insert, Range(pos, 1), bytes);
    appendRevision(rev);
}
    
void Engine::erase(const Range& range) {
    auto rev = Revision(authorId_, Revision::Operation::erase, range, nullptr);
    appendRevision(rev);
}
   
void Engine::appendRevision(Revision rev) {
    std::vector<Revision> additionals;
    for (auto& history : revisions_) {
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
    }
    
    if (!additionals.empty()) {
        revisions_.insert(revisions_.end(), additionals.begin(), additionals.end());
    }
}
  
void Engine::apply(not_null<Rope*> rope) {
    auto sorted = revisions_;
    std::sort(sorted.begin(), sorted.end(), [](auto& f, auto& s) { return f.authorId() < s.authorId(); });
    for (auto& rev : sorted) {
        rev.apply(rope);
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
