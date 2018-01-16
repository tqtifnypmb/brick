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
                        // FIXME: must create a revision belong to same author with valid revId
                        auto tail = Revision(rev.authorId(), std::numeric_limits<size_t>::max() - rev.revId(), rev.op(), Range(affectRange.maxLocation(), oldLength - rev.range().length));
                        return tail;
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
                                // FIXME: must create a revision belong to same author with valid revId
                                auto tail = Revision(rev.authorId(), std::numeric_limits<size_t>::max() - rev.revId(), rev.op(), Range(intersect.maxLocation(), revMaxLoc - intersect.maxLocation()));
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
    if (rev.authorId() == authorId_) {
        size_t maxSelfRevId = 0;
        bool found = false;
        std::for_each(revisions_.begin(), revisions_.end(), [&maxSelfRevId, authorId = authorId_, &found](const auto& r){
            if (r.authorId() == authorId) {
                maxSelfRevId = std::max(maxSelfRevId, r.revId());
                found = true;
            }
        });
        
        if (found && rev.revId() <= maxSelfRevId) {
            return {};
        } else {
            return {rev};
        }
    }
    
    std::vector<Revision> additionals;
    for (auto& history : revisions_) {
        if (rev.authorId() == history.authorId() &&
            rev.revId() <= history.revId()) {
            return {};
        }
        
        // rev from same author. Don't need to delta it
        if (rev.authorId() == history.authorId() &&
            rev.revId() > history.revId()) {
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
        additionals.push_back(rev);
    }
    
    auto locDesc = [](const Revision& lhs, const Revision& rhs) {
        return !lhs.range().before(rhs.range());
    };
    std::sort(additionals.begin(), additionals.end(), locDesc);
    
    return additionals;
}
    
Engine::Engine(size_t authorId, not_null<Rope*> rope)
    : rope_(rope)
    , authorId_(authorId)
    , revId_(0) {}
    
Engine::DeltaList Engine::insert(const CodePointList& cplist, size_t pos) {
    auto rev = Revision(authorId_, nextRevId(), Revision::Operation::insert, Range(static_cast<int>(pos), 1), cplist);
    return appendRevision(rev);
}
    
Engine::DeltaList Engine::erase(const Range& range) {
    auto rev = Revision(authorId_, nextRevId(), Revision::Operation::erase, range);
    return appendRevision(rev);
}
   
bool Engine::appendRevision(Revision rev, bool pendingRev, std::vector<Revision>* d) {
    auto origin = rev;
    auto deltas = delta(rev);
    
    std::cout<<authorId_<<std::endl;
    for (const auto& delta : deltas) {
        std::cout<<delta.authorId()<<" "<<delta.range().location<<" "<<delta.range().length<<std::endl;
    }
    std::cout<<"===="<<std::endl;
    
    for (const auto& delta : deltas) {
        if (!delta.canApply(rope_)) {
            if (!pendingRev) {
                pendingRevs_.push_back(origin);
            }
            return false;
        }
    }
    
    for (const auto& delta : deltas) {
        delta.apply(rope_);
        revisions_.push_back(delta);
    }
    
    if (d != nullptr) {
        d->insert(d->end(), deltas.begin(), deltas.end());
    }
    
    // return, if we're handling pending rev
    if (pendingRev) {
        return true;
    }
    
    auto iter = pendingRevs_.begin();
    while (iter != pendingRevs_.end()) {
        auto rev = *iter;
        bool applied = appendRevision(rev, true, d);
        if (applied) {
            iter = pendingRevs_.erase(iter);
        } else {
            ++iter;
        }
    }
    return true;
}
    
Engine::DeltaList Engine::appendRevision(Revision rev) {
    std::vector<Revision> deltas;
    appendRevision(rev, false, &deltas);
    return deltas;
}
    
Engine::DeltaList Engine::sync(const Engine& other) {
    if (other.revisions_.empty()) {
        return {};
    }

    auto validId = syncState_[other.authorId_];
    size_t latestRevId = validId;
    std::vector<Revision> deltaRevs;
    std::for_each(other.revisions_.begin(), other.revisions_.end(), [&latestRevId, &deltaRevs, validId](const auto& rev) {
        if (rev.revId() >= validId) {
            deltaRevs.push_back(rev);
            latestRevId = std::max(latestRevId, rev.revId());
        }
    });

    if (deltaRevs.empty()) {
        return {};
    }
    
    Expects(latestRevId >= validId);
    syncState_[other.authorId_] = latestRevId + 1;

    std::vector<Revision> deltas;
    for (const auto& rev : deltaRevs) {
        if (rev.authorId() == authorId_) continue;
        
        appendRevision(rev, false, &deltas);
    }
    
    return deltas;
}
    
void Engine::fastForward(const std::vector<Revision>& revs) {
    if (revs.empty()) return;
    
    decltype(syncState_) state;
    std::for_each(revs.begin(), revs.end(), [&state](const auto& rev) {
        state[rev.authorId()] = std::max(state[rev.authorId()], rev.revId());
    });
    
    for (auto& s : state) {
        auto maxId = s.second + 1;
        syncState_[s.first] = std::max(syncState_[s.first], maxId);
    }

    revisions_.insert(revisions_.end(), revs.begin(), revs.end());
//    for (const auto& rev : revs) {
//        revisions_.emplace_back(authorId_, nextRevId(), rev.op(), rev.range(), rev.cplist());
//    }
}
    
}   // namespace brick
