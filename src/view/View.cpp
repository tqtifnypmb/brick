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

using namespace gsl;

namespace brick
{
    
View::View(size_t viewId)
    : View(viewId, nullptr) {}
    
View::View(size_t viewId, span<const char> text, Range sel)
    : sel_(sel)
    , viewId_(viewId) {
    editor_ = std::make_unique<Editor>(this, text);
}
    
View::View(size_t viewId, const char* filePath)
    : viewId_(viewId){}
    
void View::scroll(size_t begRow, size_t endRow) {
    visibleRange_.first = begRow;
    visibleRange_.second = endRow;
}
 
void View::insert(span<const char> text) {
    if (sel_.length > 0) {
        editor_->erase(sel_);
        sel_.length = 0;
    }
    auto cplist = ASCIIConverter::encode(text);
    editor_->insert(cplist, sel_.location);
    sel_.offset(static_cast<int>(cplist.size()));
}
    
void View::erase() {
    editor_->erase(sel_);
}
  
std::string View::region(size_t begRow, size_t endRow) {
    return editor_->region(begRow, endRow);
}
    
}   // namespace brick
