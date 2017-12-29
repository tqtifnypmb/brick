//
//  View.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "View.h"
#include "../editor/Editor.h"
#include "../converter/Converter.h"

#include <iostream>

using namespace gsl;

namespace brick
{
    
View::View(size_t viewId)
    : viewId_(viewId) {
    editor_ = std::make_unique<Editor>(this);
}
    
View::View(size_t viewId, const detail::CodePointList& cplist, Range sel)
    : sel_(sel)
    , viewId_(viewId) {
    editor_ = std::make_unique<Editor>(this, cplist);
}
    
View::View(size_t viewId, const std::string& filePath)
    : viewId_(viewId){}
    
void View::scroll(size_t begRow, size_t endRow) {
    visibleRange_.first = begRow;
    visibleRange_.second = endRow;
    
    viewSize_ = std::max(viewSize_, endRow - begRow);
}
 
void View::insert(const detail::CodePointList& cplist) {
    if (sel_.length > 0) {
        editor_->erase(sel_);
        sel_.length = 0;
    }
    editor_->insert(cplist, sel_.location);
    sel_.offset(static_cast<int>(cplist.size()));
}
    
void View::erase() {
    editor_->erase(sel_);
    sel_.length = 0;
}
  
void View::select(Range sel) {
    sel_ = sel;
}
    
std::map<size_t, detail::CodePointList> View::regionImpl(size_t begRow, size_t endRow) {
    return editor_->region(begRow, endRow);
}
    
}   // namespace brick
