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
#include "../converter/Converter.h"

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

void Editor::adjust(const Engine::DeltaList& dlist) {
    if (dlist.empty()) return;
    
    // line index cache invalidate
    int minPos = std::numeric_limits<int>::max();
    int maxPos = 0;
    std::for_each(dlist.begin(), dlist.end(), [&minPos, &maxPos](const auto& l) {
        minPos = std::min(l.range().location, minPos);
        maxPos = std::max(l.range().maxLocation(), maxPos);
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
    
int Editor::codePointIndexToLineNum(size_t index) {
    // 1. try linesIndex_ cache first
    for (const auto& p : linesIndex_) {
        if (p.second == index) {
            return static_cast<int>(p.first);
        }
    }
    
    // 2. try find the closest index
    std::map<size_t, size_t> indexLines;
    for (const auto& p : linesIndex_) {
        indexLines[p.second] = p.first;
    }
    
    auto closest = indexLines.lower_bound(index);
    if (closest != indexLines.end()) {
        auto line = codePointIndexToLineNum(closest->first, closest->second, index);
        linesIndex_[line] = index;
        return line;
    } else if (!indexLines.empty()) {
        auto closest = indexLines.rbegin();
        auto line = codePointIndexToLineNum(closest->first, closest->second, index);
        linesIndex_[line] = index;
        return line;
    }
    
    // 3. count it from begining
    auto line = codePointIndexToLineNum(0, 0, index);
    linesIndex_[line] = index;
    return line;
}
    
namespace
{
    bool isNewLine(const CodePoint& cp) {
        char ch = static_cast<char>(cp[0]);
        return ch == '\n';
    }
}
    
int Editor::codePointIndexToLineNum(size_t initIndex, size_t initRow, size_t destIndex) {
    auto forward = initIndex < destIndex;
    auto iterator = rope_->iterator(initIndex);
    auto end = forward ? rope_->end() : rope_->begin();
    auto offset = forward ? 1 : -1;
    auto row = static_cast<int>(initRow);
    
    while (iterator != end) {
        std::advance(iterator, offset);
        if (isNewLine(*iterator)) {
            row += offset;
        }
        
        if (iterator.index() == initIndex) {
            return row;
        }
    }
    
    return row;
}
    
Editor::DeltaList Editor::convertEngineDelta(const Engine::DeltaList& deltas) {
    if (deltas.empty()) return {};
    
    Editor::DeltaList ret;
    for (const auto& delta : deltas) {
        auto begRow = codePointIndexToLineNum(delta.range().location);
        auto endIndex = delta.range().location + delta.affectLength();
        auto endRow = codePointIndexToLineNum(endIndex);
        ret.emplace_back(delta, begRow, endRow);
    }

    return ret;
}
    
Editor::DeltaList Editor::merge(const Editor& other) {
    if (rope_->empty() && engine_.revisions().empty()) {
        auto rope = Rope(*other.rope_);
        rope_->swap(rope);
        linesIndex_.clear();
        if (!other.linesIndex_.empty()) {
            linesIndex_.insert(other.linesIndex_.begin(), other.linesIndex_.end());
        }
        
        Engine::DeltaList ret;
        auto str = rope_->string();
        auto cplist = ASCIIConverter::encode(gsl::make_span(str.c_str(), str.length()));
        ret.emplace_back(engine_.authorId(), engine_.nextRevId(), Revision::Operation::insert, Range(0, static_cast<int>(rope_->size())), cplist);
        return convertEngineDelta(ret);
    } else {
        auto deltas = engine_.sync(other.engine_);
        adjust(deltas);
        return convertEngineDelta(deltas);
    }
}
    
std::map<size_t, CodePointList> Editor::region(size_t begRow, size_t endRow) {
    Expects(begRow < endRow);
    
    // 1. try to find the row closest begRow
    auto closest = linesIndex_.lower_bound(begRow);
    if (closest != linesIndex_.end()) {
        return region(closest->second, closest->first, begRow, endRow);
    }
    
    // 2. if not found, the biggest key is the one closest to begRow
    if (!linesIndex_.empty()) {
        auto closest = linesIndex_.rbegin();
        return region(closest->second, closest->first, begRow, endRow);
    }
    
    // 3. count and cache it
    return region(0, 0, begRow, endRow);
}
    
std::map<size_t, CodePointList> Editor::region(size_t initIndex, size_t initRow, size_t begRow, size_t endRow) {
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
