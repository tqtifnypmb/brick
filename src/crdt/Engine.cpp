//
//  Engine.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Engine.h"

#include <set>

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
                    case Revision::Operation::insert: {
                        if (history.prior(rev)) {
                            rev.range().offset(history.affectLength());
                        }
                        break;
                    }
                        
                    case Revision::Operation::erase: {
                        auto affectRange = Range(history.range().location, history.affectLength());
                        auto intersect = affectRange.intersection(rev.range());
                        auto oldLength = rev.range().length;
                        rev.range().length = std::max(intersect.location - rev.range().location, 0);
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
    
void Engine::insert(const CodePointList& cplist, size_t pos) {
    auto rev = Revision(authorId_, nextRevId(), Revision::Operation::insert, Range(static_cast<int>(pos), 1), cplist);
    Expects(rev.canApply(rope_));
    
    appendRevision(rev);
}
    
void Engine::erase(const Range& range) {
    auto rev = Revision(authorId_, nextRevId(), Revision::Operation::erase, range);
    Expects(rev.canApply(rope_));
    
    appendRevision(rev);
}
 
void Engine::appendRevision(Revision rev) {
    if (rev.canApply(rope_)) {
        rev.apply(rope_);
        revisions_.push_back(rev);
    } else {
        pendingRevs_.push_back(rev);
    }
    
    auto iter = pendingRevs_.begin();
    while (iter != pendingRevs_.end()) {
        auto rev = *iter;
        bool applied = appendRevision(rev, true, nullptr);
        if (applied) {
            iter = pendingRevs_.erase(iter);
        } else {
            ++iter;
        }
    }
}
    
bool Engine::appendRevision(Revision rev, bool pendingRev, std::vector<Revision>* d) {
    auto origin = rev;
    auto deltas = delta(rev);
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
    }
    revisions_.push_back(rev);
    
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
    
std::pair<Engine::DeltaList, Engine::DeltaList> Engine::sync(Engine& other) {
    return sync(other, true);
}
    
std::pair<Engine::DeltaList, Engine::DeltaList> Engine::sync(Engine& other, bool symetric) {
    if (other.revisions_.empty()) {
        return {};
    }

    // find revisions unknown by `byEngine` from `fromEngine`
    auto findUnknown = [](Engine& byEngine, std::vector<Revision>& unknown, const Engine& fromEngine) {
        auto comparator = [](const Revision& l, const Revision& r) {
            if (l.authorId() == r.authorId()) {
                return l.revId() < r.revId();
            } else {
                return l.authorId() < r.authorId();
            }
        };
        auto fromRevs = std::set<Revision, decltype(comparator)>(fromEngine.revisions().begin(), fromEngine.revisions().end(), comparator);
        auto byRevs = std::set<Revision, decltype(comparator)>(byEngine.revisions().begin(), byEngine.revisions().end(), comparator);
        std::set_difference(fromRevs.begin(),
                            fromRevs.end(),
                            byRevs.begin(),
                            byRevs.end(),
                            std::back_inserter(unknown),
                            comparator);
//        return;
//        auto lastState = byEngine.sync_state_.find(fromEngine.authorId_);
//        if (lastState == byEngine.sync_state_.end()) {
//            auto comparator = [](const Revision& l, const Revision& r) {
//                if (l.authorId() == r.authorId()) {
//                    return l.revId() < r.revId();
//                } else {
//                    return l.authorId() < r.authorId();
//                }
//            };
//
//            auto fromRevs = std::set<Revision, decltype(comparator)>(fromEngine.revisions().begin(), fromEngine.revisions().end(), comparator);
//            auto byRevs = std::set<Revision, decltype(comparator)>(byEngine.revisions().begin(), byEngine.revisions().end(), comparator);
//            std::set_difference(fromRevs.begin(),
//                                fromRevs.end(),
//                                byRevs.begin(),
//                                byRevs.end(),
//                                std::back_inserter(unknown),
//                                comparator);
//        } else {
//            auto lastAuthorId = lastState->second.first;
//            auto lastRevId = lastState->second.second;
//            auto lastknown = std::find_if(fromEngine.revisions().begin(), fromEngine.revisions().end(), [lastAuthorId, lastRevId](const auto& rev) {
//                return rev.authorId() == lastAuthorId && rev.revId() == lastRevId;
//            });
//
//            if (lastknown != fromEngine.revisions().end()) {
//                std::advance(lastknown, 1);
//                unknown.assign(lastknown, fromEngine.revisions().end());
//            }
//        }
    };
    
    std::vector<Revision> unknownBySelf;
    std::vector<Revision> unknownByOther;
    findUnknown(*this, unknownBySelf, other);
    findUnknown(other, unknownByOther, *this);
    
    
    // 1. if self know everything about other, nothing needs to do
    if (unknownBySelf.empty()) {
        return {};
    }
    
    std::vector<Revision> deltas;
    std::vector<Revision> other_deltas;
    
    // FIXME: if there's a in-flight request arrived after two engine synced
    // then `unknownByOther` will be empty and that reqeust would be incorrectly
    // treated as if it's made after two engines are synced -- no delta is made.
    // that's wrong.
    // Need a mechanism to identify whether a request is made in synced-state or not.
    // Just because `unknownByOther` is empty doesn't means request is made in synced-state;
//    if (unknownByOther.empty()) {
//        auto lastSyncState = other.sync_state_.find(authorId_);
//        Expects(lastSyncState != other.sync_state_.end());
//        auto lastknown = std::find_if(revisions_.begin(), revisions_.end(), [lastAuthorId = lastSyncState->second.first, lastRevId = lastSyncState->second.second](const auto& rev) {
//            return rev.authorId() == lastAuthorId && rev.revId() == lastRevId;
//        });
//        if (lastknown != revisions_.end()) {
//            std::advance(lastknown, 1);
//            unknownByOther.assign(lastknown, revisions_.end());
//        }
//    }
    
    // 2. other and self are already in sync state
    //    we can just apply what self don't know without
    //    calculating delta.
    // Note: We assume that all unknownBySelf revisions are
    //       applied after two engines have reached synced state.
    if (unknownByOther.empty()) {
        for (const auto& rev : unknownBySelf) {
            if (rev.canApply(rope_)) {
                rev.apply(rope_);
                revisions_.push_back(rev);
                deltas.push_back(rev);
            } else {
                pendingRevs_.push_back(rev);
            }
        }
    }
    
    // 3. otherwise, we need to calculate delta of each
    //    revision and apply it
    else {
        for (auto r : unknownBySelf) {
            std::vector<Revision> subDeltas;
            for (const Revision& history : unknownByOther) {
                auto d = delta(history, r);
                if (d.valid()) {
                    subDeltas.push_back(d);
                }
            }
            
            if (r.valid()) {
                subDeltas.push_back(r);
            }
            
            auto canApply = true;
            for (const auto& rev : subDeltas) {
                if (!rev.canApply(rope_)) {
                    canApply = false;
                    break;
                }
            }
            
            if (!canApply) {
                pendingRevs_.push_back(r);
                continue;
            }
            
            auto locDesc = [](const Revision& lhs, const Revision& rhs) {
                return !lhs.range().before(rhs.range());
            };
            std::sort(subDeltas.begin(), subDeltas.end(), locDesc);
            for (const auto& rev : subDeltas) {
                rev.apply(rope_);
            }
            
            deltas.insert(deltas.end(), subDeltas.begin(), subDeltas.end());
        }
    }
    
    if (symetric) {
        auto d = other.sync(*this, false);
        Expects(d.second.empty());
        
        other_deltas = std::move(d.first);
    }
    revisions_.insert(revisions_.end(), unknownBySelf.begin(), unknownBySelf.end());
    
    // 4. apply pending revisions
    auto iter = pendingRevs_.begin();
    while (iter != pendingRevs_.end()) {
        auto rev = *iter;
        bool applied = appendRevision(rev, true, &deltas);
        if (applied) {
            iter = pendingRevs_.erase(iter);
        } else {
            ++iter;
        }
    }
    
    return {deltas, other_deltas};
}
    
void Engine::fastForward(const std::vector<Revision>& revs) {
    if (revs.empty()) return;

    revisions_.insert(revisions_.end(), revs.begin(), revs.end());
}
    
}   // namespace brick
