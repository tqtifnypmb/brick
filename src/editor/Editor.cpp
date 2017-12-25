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

using namespace brick::detail;

namespace brick
{
    
Editor::Editor(View* view, const CodePointList& cplist)
    : view_(view)
    , rope_()
    , engine_(view->viewId(), rope_.get()) {
    if (!cplist.empty()) {
        rope_->insert(cplist, 0);
    }
}
   
void Editor::insert(const CodePointList &cplist, size_t pos) {
    engine_.insert(cplist, pos);
}
    
std::string Editor::region(size_t begRow, size_t endRow) {
    // 1. try to find the row closest begRow
    auto found = linesIndex_.lower_bound(begRow);
    if (found != linesIndex_.end()) {
        auto backward = found->first > begRow;
        return region(found->second, found->first, begRow, endRow, !backward);
    }
    
    // 2. if not found, the biggest key is the one closest to begRow
    if (!linesIndex_.empty()) {
        auto last = linesIndex_.rbegin();
        return region(last->second, last->first, begRow, endRow, true);
    }
    
    // 3. count and cache it
    return region(0, 0, begRow, endRow, true);
}
   
namespace
{
    bool isNewLine(const CodePoint& cp) {
        char ch = static_cast<char>(cp[0]);
        return ch == '\n';
    }
}
    
std::string Editor::region(size_t initIndex, size_t initRow, size_t begRow, size_t endRow, bool forward) {
    auto iterator = rope_->iterator(initIndex);
    RopeIter endIter;
    int offset;
    if (forward) {
        offset = 1;
        endIter = rope_->begin();
    } else {
        offset = -1;
        endIter = rope_->end();
    }
    
    auto interval = begRow - initRow;
    while (interval > 0 && iterator != endIter) {
        auto cp = *iterator;
        if (isNewLine(cp)) {
            interval -= 1;
        }
        std::advance(iterator, offset);
    }
    
    if (interval != 0) {
        // FIXME: - throw ?
    }
    
    auto begIndex = iterator.index() + iterator.offset();
    auto numOfRow = endRow - begRow;
    while (numOfRow > 0 && iterator != endIter) {
        auto cp = *iterator;
        if (isNewLine(cp)) {
            numOfRow -= 1;
        }
        std::advance(iterator, offset);
    }
    
    if (numOfRow != 0) {
        // FIXME: - throw ?
    }
    
    if (begRow - initRow >= view_->viewSize()) {
        linesIndex_[begRow] = begIndex;
    }
    
    size_t endIndex = iterator.index() + iterator.offset();
    auto range = Range(static_cast<int>(begIndex), static_cast<int>(endIndex));
    return rope_->region(range);
}
    
}
