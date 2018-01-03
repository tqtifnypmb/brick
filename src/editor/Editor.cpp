//
//  editor.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Editor.h"
#include "../rope/Rope.h"
#include "../view/View.h"

#include <limits>
#include <iostream>

using namespace brick::detail;

namespace brick
{
    
Editor::Editor(View* view, const CodePointList& cplist)
    : view_(view)
    , rope_(std::make_unique<Rope>())
    , engine_(view->viewId(), rope_.get()) {
    if (!cplist.empty()) {
        rope_->insert(cplist, 0);
    }
}
   
Editor::Editor(View* view)
    : Editor(view, CodePointList()) {}
    
void Editor::insert(const CodePointList &cplist, size_t pos) {
    auto delta = engine_.insert(cplist, pos);
    adjust(delta);
}
    
void Editor::erase(Range range) {
    auto delta = engine_.erase(range);
    adjust(delta);
}
    
void Editor::undo() {
}

void Editor::adjust(const Engine::Delta& delta) {
    if (delta.empty()) return;
   
    // adjust view selection
    // FIXME: Doesn't this should happend on client side?
    auto affected = view_->selection();
    for (const auto& d : delta) {
        if (!affected.before(d.first)) {
            if (affected.intersect(d.first)) {
                affected.location = d.first.maxLocation();
                affected.length = 1;
            } else {
                if (d.second == Revision::Operation::insert) {
                    affected.offset(d.first.length);
                } else {
                    affected.offset(-d.first.length);
                }
            }
        }
    }
    
    auto sel = view_->selection();
    if (!sel.before(affected)) {
        if (sel.intersect(affected)) {
            
        } else {
            sel.offset(affected.length);
        }
        view_->select(sel);
    }
    
    // line index cache invalidate
    int minPos = std::numeric_limits<int>::max();
    int maxPos = 0;
    std::for_each(delta.begin(), delta.end(), [&minPos, &maxPos](const auto& l) {
        minPos = std::min(l.first.location, minPos);
        maxPos = std::max(l.first.maxLocation(), maxPos);
    });
    
    auto iter = linesIndex_.begin();
    while (iter != linesIndex_.end()) {
        if (iter->second >= minPos && iter->second < maxPos) {
            iter = linesIndex_.erase(iter);
        } else {
            ++iter;
        }
    }
}
    
Engine::Delta Editor::merge(const Editor& other) {
    if (rope_->empty() && engine_.revisions().empty()) {
        auto rope = Rope(*other.rope_);
        rope_->swap(rope);
        linesIndex_.clear();
        if (!other.linesIndex_.empty()) {
            linesIndex_.insert(other.linesIndex_.begin(), other.linesIndex_.end());
        }
        
        Engine::Delta ret;
        ret.push_back(std::make_pair(Range(0, static_cast<int>(rope.size())), Revision::Operation::insert));
        return ret;
    } else {
        return engine_.sync(other.engine_);
    }
}
    
std::map<size_t, CodePointList> Editor::region(size_t begRow, size_t endRow) {
    // 1. try to find the row closest begRow
    auto found = linesIndex_.lower_bound(begRow);
    if (found != linesIndex_.end()) {
        return region(found->second, found->first, begRow, endRow);
    }
    
    // 2. if not found, the biggest key is the one closest to begRow
    if (!linesIndex_.empty()) {
        auto last = linesIndex_.rbegin();
        return region(last->second, last->first, begRow, endRow);
    }
    
    // 3. count and cache it
    return region(0, 0, begRow, endRow);
}
   
namespace
{
    bool isNewLine(const CodePoint& cp) {
        char ch = static_cast<char>(cp[0]);
        return ch == '\n';
    }
}
    
std::map<size_t, CodePointList> Editor::region(size_t initIndex, size_t initRow, size_t begRow, size_t endRow ) {
    // 1. find begRow
    auto iterator = rope_->iterator(initIndex);
    RopeIter endIter;
    int offset;
    auto forward = begRow > initRow;
    if (forward) {
        offset = 1;
        endIter = rope_->end();
    } else {
        offset = -1;
        endIter = rope_->begin();
    }
    
    auto interval = begRow - initRow;
    while (interval > 0 && iterator != endIter) {
        auto cp = *iterator;
        if (isNewLine(cp)) {
            interval -= 1;
        }
        std::advance(iterator, offset);
    }
    
    // begRow out of range
    if (interval != 0) {
        return {};
    }
    
    // found valid index for begRow, cache it if neccessary
    auto begIndex = iterator.index() + iterator.offset();
    if (begRow - initRow >= view_->viewSize()) {
        linesIndex_[begRow] = begIndex;
    }
    
    // 2. find endRow and collect line results
    auto numOfRow = endRow - begRow;;
    size_t currentRow = 0;
    std::map<size_t, CodePointList> ret;
    CodePointList line;
    while (numOfRow > 0 && iterator != rope_->end()) {
        auto cp = *iterator;
        if (isNewLine(cp)) {
            ret[begRow + currentRow] = line;
            numOfRow -= 1;
            currentRow += 1;
            line.erase(line.begin(), line.end());
        } else {
            line.push_back(cp);
        }
        std::advance(iterator, 1);
    }
    
    if (!line.empty()) {
        ret[begRow + currentRow] = line;
    }
    
    if (numOfRow == 0) {
        // found valid index for endRow, cache it if neccessary
        size_t endIndex = iterator.index() + iterator.offset();
        
        if (endRow - begRow >= view_->viewSize()) {
            linesIndex_[endRow] = endIndex;
        }
    }
    
    return ret;
}
    
}
