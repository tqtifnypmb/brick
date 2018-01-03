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
    
View::~View() {
    if (hasParent()) {
        parent_->removeChild(this);
    }
}
    
View::View(size_t viewId, UpdateCb cb)
    : viewId_(viewId)
    , parent_(nullptr)
    , update_cb_(cb) {
    editor_ = std::make_unique<Editor>(this);
}
    
View::View(size_t viewId, const detail::CodePointList& cplist, UpdateCb cb, Range sel)
    : sel_(sel)
    , viewId_(viewId)
    , parent_(nullptr)
    , update_cb_(cb) {
    editor_ = std::make_unique<Editor>(this, cplist);
}
    
View::View(size_t viewId, const std::string& filePath, UpdateCb cb)
    : viewId_(viewId)
    , filePath_(filePath)
    , parent_(nullptr)
    , update_cb_(cb) {
    // FIXME: - read file content
    editor_ = std::make_unique<Editor>(this);
}
    
View::View(size_t viewId, View* parent, UpdateCb cb)
    : View(viewId, cb) {
    parent_ = parent;
    parent_->children_.push_back(this);
    editor_->merge(*parent->editor_);
}
    
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
    update(this);
}
    
void View::erase() {
    editor_->erase(sel_);
    sel_.length = 0;
    update(this);
}
  
void View::select(Range sel) {
    sel_ = sel;
}
    
std::map<size_t, detail::CodePointList> View::regionImpl(size_t begRow, size_t endRow) {
    return editor_->region(begRow, endRow);
}
    
void View::save()  {
    if (filePath_.empty()) {
        throw std::invalid_argument("Invalid file path");
    }
    save(filePath_);
}
    
void View::save(const std::string& filePath) {
}
  
void View::update(View* src) {
    Engine::Delta deltas;
    
    // 1. merve revision
    if (hasChildren() || hasParent()) {
        if (hasParent() && parent_ != src) {
            auto d = parent_->editor_->merge(*editor_);
            deltas.insert(deltas.end(), d.begin(), d.end());
            parent_->update(this);
        }
        
        for (auto child : children_) {
            if (child == src) continue;
            
            auto d = child->editor_->merge(*editor_);
            deltas.insert(deltas.end(), d.begin(), d.end());
            child->update(this);
        }
    }
    //FIXME: consider undo
    editor_->clearRevisions();
    
    // 2. update view
    if (src != this) {
        update_cb_(viewId_, deltas);
    }
}
    
}   // namespace brick
