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
    : seletion_(sel)
    , viewId_(viewId) {
    editor_ = std::make_unique<Editor>(this, text);
}
    
View::View(size_t viewId, const char* filePath)
    : viewId_(viewId){
    
}
    
void View::scroll(Range visibleRange) {
    visibleRange_ = visibleRange;
}
 
void View::insert(span<const char> text) {
    if (seletion_.length > 0) {
        editor_->erase(seletion_);
        seletion_.length = 0;
    }
    auto cplist = ASCIIConverter::encode(text);
    editor_->insert(cplist, seletion_.location);
    seletion_.offset(cplist.size());
}
    
void View::erase() {
    //editor_->erase(range);
}
    
}
